#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 64
struct ClientInfo {
    int fd;
    char id[32];
};
struct ClientInfo clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
void broadcast_message(int sender_fd, char *sender_id, char *msg) {
    char send_buf[1024];
    sprintf(send_buf, "%s: %s\n", sender_id, msg);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].fd != sender_fd) {
            send(clients[i].fd, send_buf, strlen(send_buf), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
void remove_client(int fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].fd == fd) {
            for (int j = i; j < num_clients - 1; j++) {
                clients[j] = clients[j + 1];
            }
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
void* client_handler(void *arg) {
    int client_fd = *(int*)arg;
    free(arg);
    char buf[512];
    char client_id[32] = "";
    char client_name[64] = "";
    int logged_in = 0;
    char *prompt = "Vui long dang nhap theo cu phap 'client_id: client_name'\n";
    send(client_fd, prompt, strlen(prompt), 0);
    while (!logged_in) {
        int bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) {
            close(client_fd);
            return NULL;
        }
        buf[bytes] = '\0';
        buf[strcspn(buf, "\r\n")] = 0;
        int parsed = sscanf(buf, "%31[^:]: %63s", client_id, client_name);
        if (parsed == 2) {
            logged_in = 1;
            char *success = ">> Dang nhap thanh cong! Ban co the chat.\n";
            send(client_fd, success, strlen(success), 0);
            pthread_mutex_lock(&clients_mutex);
            clients[num_clients].fd = client_fd;
            strcpy(clients[num_clients].id, client_id);
            num_clients++;
            pthread_mutex_unlock(&clients_mutex);
            printf(">> Client [%s] da dang nhap.\n", client_id);
        } else {
            char *err = ">> Sai cu phap! Mau hop le: 'alice: AliceNguyen'\n";
            send(client_fd, err, strlen(err), 0);
        }
    }
    while (1) {
        int bytes = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) {
            printf(">> Client [%s] da ngat ket noi.\n", client_id);
            remove_client(client_fd);
            break;
        }   
        buf[bytes] = '\0';
        buf[strcspn(buf, "\r\n")] = 0;
        broadcast_message(client_fd, client_id, buf);
    }
    close(client_fd);
    return NULL;
}
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);
    printf("Chat Server Multithread dang lang nghe o cong 9000...\n");
    while (1) {
        int client_fd = accept(listener, NULL, NULL);
        if (client_fd == -1) continue;
        int *client_ptr = malloc(sizeof(int));
        *client_ptr = client_fd;
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_ptr);
        pthread_detach(tid);
    }
    return 0;
}