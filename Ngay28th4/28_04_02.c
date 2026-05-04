#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Cu phap: %s <port> <remote_ip> <remote_port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    char *remote_ip = argv[2];
    int remote_port = atoi(argv[3]);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("Loi tao socket!\n");
        return 1;
    }

    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        printf("Loi bind! Cong %d co the dang bi su dung.\n", port);
        return 1;
    }

    struct sockaddr_in remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(remote_ip);
    remote_addr.sin_port = htons(remote_port);
    printf("--- Ung dung UDP Chat P2P ---\n");
    printf("Lang nghe o cong: %d | Chat voi: %s:%d\n", port, remote_ip, remote_port);
    printf("Go tin nhan cua ban roi an Enter (go 'exit' de thoat)...\n\n");
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(sock, &fdread);
    int max_fd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;
    char buf[1024];
    while (1) {
        fdtest = fdread;
        int ret = select(max_fd + 1, &fdtest, NULL, NULL, NULL);
        if (ret < 0) break;
        if (FD_ISSET(STDIN_FILENO, &fdtest)) {
            fgets(buf, sizeof(buf), stdin);
            if (strncmp(buf, "exit", 4) == 0) break;
            sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
        }
        if (FD_ISSET(sock, &fdtest)) {
            int bytes = recvfrom(sock, buf, sizeof(buf) - 1, 0, NULL, NULL);
            if (bytes > 0) {
                buf[bytes] = '\0';
                printf("\r[Remote]: %s", buf); 
            }
        }
    }
    close(sock);
    return 0;
}
