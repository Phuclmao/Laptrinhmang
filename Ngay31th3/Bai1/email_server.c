#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <ctype.h>

struct client_info {
    int fd;
    int state;
    char name[256];
    char mssv[64];
};
void remove_client(struct client_info clients[], int *num, int fd) {
    for (int i = 0; i < *num; i++) {
        if (clients[i].fd == fd) {
            for (int j = i; j < *num - 1; j++) {
                clients[j] = clients[j + 1];
            }
            (*num)--;
            break;
        }
    }
}
void generate_email(char *fullname, char *mssv, char *email) {
    char *words[20];
    int count = 0;
    char *token = strtok(fullname, " ");
    while (token != NULL) {
        words[count++] = token;
        token = strtok(NULL, " ");
    }

    if (count == 0) {
        strcpy(email, "Loi: Khong nhan dien duoc ten!\n");
        return;
    }

    char formatted[256] = "";
    strcat(formatted, words[count - 1]);
    strcat(formatted, ".");
    for (int i = 0; i < count - 1; i++) {
        int len = strlen(formatted);
        formatted[len] = words[i][0];
        formatted[len + 1] = '\0';
    }
    for (int i = 0; formatted[i]; i++) {
        formatted[i] = tolower(formatted[i]);
    }
    char *short_mssv = mssv;
    if (strlen(mssv) >= 2 && mssv[0] == '2' && mssv[1] == '0') {
        short_mssv = mssv + 2;
    }
    sprintf(email, "-> Email cua ban la: %s%s@sis.hust.edu.vn\n", formatted, short_mssv);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Cu phap: %s <Cổng>\n", argv[0]);
        return 1;
    }
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    printf("Non-blocking Email Server dang lang nghe o cong %s...\n", argv[1]);
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);
    struct client_info clients[64];
    int num_clients = 0;
    char buf[1024];
    while (1) {
        fdtest = fdread;
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret < 0) break;
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &fdtest)) {
                if (i == listener) {
                    int new_client = accept(listener, NULL, NULL);
                    printf(">> Co client moi ket noi (ID: %d)\n", new_client);
                    FD_SET(new_client, &fdread);
                    clients[num_clients].fd = new_client;
                    clients[num_clients].state = 1;
                    num_clients++;
                    char *msg = "Vui long nhap Ho va Ten (VD: Nguyen Van Anh): ";
                    send(new_client, msg, strlen(msg), 0);
                }
                else {
                    int bytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (bytes <= 0) {
                        printf(">> Client %d da ngat ket noi\n", i);
                        close(i);
                        FD_CLR(i, &fdread);
                        remove_client(clients, &num_clients, i);
                    } else {
                        buf[bytes] = '\0';
                        buf[strcspn(buf, "\r\n")] = 0;
                        for (int j = 0; j < num_clients; j++) {
                            if (clients[j].fd == i) {
                                if (clients[j].state == 1) { 
                                    strcpy(clients[j].name, buf);
                                    clients[j].state = 2;
                                    char *msg = "Vui long nhap MSSV (VD: 20201234): ";
                                    send(i, msg, strlen(msg), 0);
                                } else if (clients[j].state == 2) { 
                                    strcpy(clients[j].mssv, buf);
                                    char email[256];
                                    generate_email(clients[j].name, clients[j].mssv, email);
                                    send(i, email, strlen(email), 0);
                                    close(i);
                                    FD_CLR(i, &fdread);
                                    remove_client(clients, &num_clients, i);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}