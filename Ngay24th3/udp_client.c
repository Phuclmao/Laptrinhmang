#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[ ]) {
    if (argc != 3) {
        printf("Cu phap: %s <IP_Server> <Cổng>\n", argv[ 0 ]);
        return 1;
    }

    // Tạo socket UDP
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Cấu hình địa chỉ đích (Server)
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[ 1 ]);
    addr.sin_port = htons(atoi(argv[ 2 ]));

    char buf[ 4096 ];
    printf("--- Ung dung Chat UDP ---\n");
    
    while (1) {
        printf("Ban: ");
        fgets(buf, sizeof(buf), stdin);
        if (strncmp(buf, "exit", 4) == 0) break;

        // Gửi thẳng gói tin sang Server không cần connect() (Đã xóa ký tự thừa ở cuối)
        sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));

        // Đợi Server (udp_echo) dội lại dữ liệu
        struct sockaddr_in server_addr; // KHẮC PHỤC LỖI: Bổ sung khai báo biến server_addr
        socklen_t server_addr_len = sizeof(server_addr);
        
        int ret = recvfrom(sender, buf, sizeof(buf) - 1, 0, 
                           (struct sockaddr *)&server_addr, &server_addr_len);

        if (ret > 0) {
            buf[ ret ] = '\0';
            printf("Echo tu Server: %s", buf);
        }
    }

    close(sender);
    return 0;
}