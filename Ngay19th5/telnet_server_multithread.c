#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

int check_login(char *user, char *pass) {
    FILE *f = fopen("database.txt", "r");
    if (!f) return 0;     
    char f_user[64], f_pass[64];
    while (fscanf(f, "%s %s", f_user, f_pass) == 2) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f); 
            return 1; 
        }
    }
    fclose(f);
    return 0;
}
void* client_handler(void *arg) {
    int client = *(int*)arg;
    free(arg);   
    char *msg = "Vui long nhap tai khoan (Cach nhau boi dau cach):\n";
    send(client, msg, strlen(msg), 0);
    char buf[1024];
    int logged_in = 0;
    while (1) {
        int bytes = recv(client, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) {
            printf(">> Client %d da ngat ket noi.\n", client);
            break; 
        }
        buf[bytes] = '\0';
        buf[strcspn(buf, "\r\n")] = 0;
        if (logged_in == 0) {
            char user[64], pass[64];
            if (sscanf(buf, "%s %s", user, pass) == 2 && check_login(user, pass)) {
                logged_in = 1;
                char *ok = ">> Dang nhap thanh cong! Ban co the nhap lenh:\n";
                send(client, ok, strlen(ok), 0);
            } else {
                char *err = ">> Tai khoan hoac mat khau sai. Vui long thu lai:\n";
                send(client, err, strlen(err), 0);
            }
        } 
        else {
            char cmd[1100];
            char out_filename[64];
            sprintf(out_filename, "out_%d.txt", client); 
            sprintf(cmd, "%s > %s", buf, out_filename);
            system(cmd);
            FILE *f = fopen(out_filename, "r");
            if (f) {
                char out_buf[1024];
                int has_out = 0;
                while (fgets(out_buf, sizeof(out_buf), f)) {
                    send(client, out_buf, strlen(out_buf), 0);
                    has_out = 1;
                }
                if (!has_out) {
                    char *no_res = "(Lenh khong tra ve ket qua)\n";
                    send(client, no_res, strlen(no_res), 0);
                }
                fclose(f);
                remove(out_filename);
            }
        }
    }
    
    close(client);
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
    printf("Telnet Server Multithread dang lang nghe o cong 9000...\n");
    while(1){
        int client_fd = accept(listener, NULL, NULL);
        if (client_fd == -1) continue;
        printf(">> Co client moi ket noi (ID: %d)\n", client_fd);
        int *client_ptr = malloc(sizeof(int));
        *client_ptr = client_fd;
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_ptr);
        pthread_detach(tid);
    }
    return 0;
}