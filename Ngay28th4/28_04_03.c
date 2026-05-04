#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 9000
#define MAX_CLIENTS 64
#define MAX_TOPICS_PER_CLIENT 10
#define TOPIC_LEN 32

struct client_info {
    int fd;
    char topics[MAX_TOPICS_PER_CLIENT][TOPIC_LEN];
    int topic_count;
};

void subscribe_topic(struct client_info *client, char *topic) {
    for (int i = 0; i < client->topic_count; i++) {
        if (strcmp(client->topics[i], topic) == 0) {
            char *msg = ">> Ban da dang ky chu de nay roi.\n";
            send(client->fd, msg, strlen(msg), 0);
            return;
        }
    }
    if (client->topic_count < MAX_TOPICS_PER_CLIENT) {
        strcpy(client->topics[client->topic_count], topic);
        client->topic_count++;
        char msg[128];
        sprintf(msg, ">> Dang ky thanh cong chu de: %s\n", topic);
        send(client->fd, msg, strlen(msg), 0);
        printf(">> Client %d dang ky chu de: %s\n", client->fd, topic);
    } else {
        char *msg = ">> Da vuot qua so luong chu de toi da.\n";
        send(client->fd, msg, strlen(msg), 0);
    }
}

void unsubscribe_topic(struct client_info *client, char *topic) {
    for (int i = 0; i < client->topic_count; i++) {
        if (strcmp(client->topics[i], topic) == 0) {
            for (int j = i; j < client->topic_count - 1; j++) {
                strcpy(client->topics[j], client->topics[j + 1]);
            }
            client->topic_count--;
            char msg[128];
            sprintf(msg, ">> Huy dang ky thanh cong chu de: %s\n", topic);
            send(client->fd, msg, strlen(msg), 0);
            printf(">> Client %d huy dang ky chu de: %s\n", client->fd, topic);
            return;
        }
    }
    char *msg = ">> Ban chua dang ky chu de nay.\n";
    send(client->fd, msg, strlen(msg), 0);
}

void publish_message(struct pollfd fds[], struct client_info clients[], int nfds, int sender_fd, char *topic, char *msg) {
    char out_buf[1024];
    sprintf(out_buf, "[%s]: %s\n", topic, msg);
    int receiver_count = 0;
    for (int i = 1; i < nfds; i++) {
        for (int j = 0; j < clients[i].topic_count; j++) {
            if (strcmp(clients[i].topics[j], topic) == 0) {
                send(clients[i].fd, out_buf, strlen(out_buf), 0);
                receiver_count++;
                break;
            }
        }
    }
    printf(">> Client %d da gui tin nhan vao '%s' (Tiep can: %d clients)\n", sender_fd, topic, receiver_count);
}

void remove_client(struct pollfd fds[], struct client_info clients[], int *nfds, int index) {
    close(fds[index].fd);
    for (int i = index; i < *nfds - 1; i++) {
        fds[i] = fds[i + 1];
        clients[i] = clients[i + 1];
    }
    (*nfds)--;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Loi bind! Cong %d co the dang bi chiem.\n", PORT);
        return 1;
    }
    listen(listener, 5);
    printf("Pub/Sub Server dang lang nghe o cong %d...\n", PORT);
    struct pollfd fds[MAX_CLIENTS];
    struct client_info clients[MAX_CLIENTS];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    char buf[1024];
    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) break;
        if (fds[0].revents & POLLIN) {
            int new_client = accept(listener, NULL, NULL);
            printf(">> Co client moi ket noi (ID: %d)\n", new_client);
            fds[nfds].fd = new_client;
            fds[nfds].events = POLLIN;
            clients[nfds].fd = new_client;
            clients[nfds].topic_count = 0; 
            nfds++;
            char *welcome = "Huong dan: SUB <topic> | UNSUB <topic> | PUB <topic> <msg>\n";
            send(new_client, welcome, strlen(welcome), 0);
        }
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & (POLLIN | POLLERR)) {
                int bytes = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
                if (bytes <= 0) {
                    printf(">> Client %d da thoat\n", fds[i].fd);
                    remove_client(fds, clients, &nfds, i);
                    i--; 
                } else {
                    buf[bytes] = '\0';
                    buf[strcspn(buf, "\r\n")] = 0;
                    char cmd[16] = "", topic[TOPIC_LEN] = "", msg[512] = "";
                    int parsed = sscanf(buf, "%15s %31s %511[^\n]", cmd, topic, msg);

                    if (strcmp(cmd, "SUB") == 0 && parsed >= 2) {
                        subscribe_topic(&clients[i], topic);
                    } 
                    else if (strcmp(cmd, "UNSUB") == 0 && parsed >= 2) {
                        unsubscribe_topic(&clients[i], topic);
                    } 
                    else if (strcmp(cmd, "PUB") == 0 && parsed >= 3) {
                        publish_message(fds, clients, nfds, fds[i].fd, topic, msg);
                    } 
                    else {
                        char *err = ">> Sai cu phap! Mau: SUB <topic> | UNSUB <topic> | PUB <topic> <msg>\n";
                        send(fds[i].fd, err, strlen(err), 0);
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}
