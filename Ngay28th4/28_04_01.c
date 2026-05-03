#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

void encrypt_string(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = (str[i] == 'z') ? 'a' : str[i] + 1;
        } 
        else if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = (str[i] == 'Z') ? 'A' : str[i] + 1;
        } 
        else if (str[i] >= '0' && str[i] <= '9') {
            str[i] = '9' - (str[i] - '0');
        }
    }
}

void remove_client(struct pollfd fds[], int *nfds, int index) {
    close(fds[index].fd);
    for (int i = index; i < *nfds - 1; i++) {
        fds[i] = fds[i + 1];
    }
    (*nfds)--;
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

    struct pollfd fds[64];
    int nfds = 1;
    fds[0].fd = listener;
    fds[0].events = POLLIN;

    printf("Encrypt Server dang lang nghe tren cong %s...\n", argv[1]);

    char buf[1024];

    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) break;
        if (fds[0].revents & POLLIN) {
            int new_client = accept(listener, NULL, NULL);
            char welcome_msg[256];
            sprintf(welcome_msg, "Xin chao. Hien co %d clients dang ket noi.\n", nfds);
            send(new_client, welcome_msg, strlen(welcome_msg), 0);
            fds[nfds].fd = new_client;
            fds[nfds].events = POLLIN;
            nfds++;
            
            printf(">> Client moi ket noi (ID: %d).\n", new_client);
        }
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & (POLLIN | POLLERR)) {
                int bytes = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
                if (bytes <= 0) {
                    printf(">> Client ID %d da thoat.\n", fds[i].fd);
                    remove_client(fds, &nfds, i);
                    i--;
                } else {
                    buf[bytes] = '\0';
                    buf[strcspn(buf, "\r\n")] = 0;
                    if (strncmp(buf, "exit", 4) == 0) {
                        char *goodbye = "Tam biet.\n";
                        send(fds[i].fd, goodbye, strlen(goodbye), 0);
                        printf(">> Client ID %d gui exit. Da ngat ket noi.\n", fds[i].fd);
                        remove_client(fds, &nfds, i);
                        i--;
                    } else {
                        encrypt_string(buf);
                        strcat(buf, "\n"); 
                        send(fds[i].fd, buf, strlen(buf), 0);
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}