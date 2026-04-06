#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Cu phap: %s <port_s> <ip_d> <port_d>\n", argv[0]);
        return 1;
    }
    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("Loi tao socket!\n");
        return 1;
    }
    struct sockaddr_in recv_addr;
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_addr.sin_port = htons(port_s);
    if (bind(sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        printf("Loi bind! Cong %d co the dang bi chiem.\n", port_s);
        return 1;
    }
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(ip_d);
    dest_addr.sin_port = htons(port_d);
    printf("--- Ung dung UDP Chat Non-blocking ---\n");
    printf("[Thong tin] Ban lang nghe o cong: %d | Nhan tin toi: %s:%d\n", port_s, ip_d, port_d);
    printf("Go tin nhan cua ban va an Enter (go 'exit' de thoat)...\n\n");
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
        if (FD_ISSET(sock, &fdtest)) {
            struct sockaddr_in sender_addr;
            socklen_t sender_len = sizeof(sender_addr);
            int bytes = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&sender_addr, &sender_len);
            if (bytes > 0) {
                buf[bytes] = '\0';
                printf("\r[Nguoi la]: %s", buf);
            }
        }
        if (FD_ISSET(STDIN_FILENO, &fdtest)) {
            fgets(buf, sizeof(buf), stdin);
            if (strncmp(buf, "exit", 4) == 0) break;
            sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        }
    }
    close(sock);
    return 0;
}