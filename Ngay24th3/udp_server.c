#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[ ]) {
    if (argc != 2) {
        printf("Cu phap: %s <Cổng>\n", argv[ 0 ]);
        return 1;
    }

    // 1. Tạo socket chuẩn UDP (Không còn các ký tự trích dẫn thừa ở cuối dòng)
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (receiver == -1) {
        printf("Khong tao duoc socket UDP!\n");
        return 1;
    }

    // 2. Cấu hình địa chỉ và gắn với cổng
    struct sockaddr_in addr; // KHẮC PHỤC LỖI: Đã bổ sung lại dòng khai báo biến addr
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[ 1 ]));

    if (bind(receiver, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("Bind that bai!\n");
        return 1;
    }
    
    printf("UDP Echo Server dang lang nghe tai cong %s...\n", argv[ 1 ]);

    // 3. Vòng lặp nhận và phản hồi dữ liệu
    char buf[ 4096 ];
    struct sockaddr_in client_addr; 
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        // Nhận dữ liệu bằng recvfrom (Không còn ký tự thừa ở cuối dòng)
        int ret = recvfrom(receiver, buf, sizeof(buf) - 1, 0, 
                           (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (ret > 0) {
            buf[ ret ] = '\0';
            printf("Nhan duoc tu client: %s", buf);

            // Phản hồi lại client
            sendto(receiver, buf, ret, 0, 
                   (struct sockaddr *)&client_addr, client_addr_len);
        }
    }

    close(receiver);
    return 0;
}