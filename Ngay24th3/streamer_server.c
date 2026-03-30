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

    // Khoi tao socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[ 1 ]));

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    printf("Server dang doi ket noi o cong %s...\n", argv[ 1 ]);

    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;
        printf("Co client moi ket noi!\n");

        char buf[ 4096 ];
        char temp[ 8192 ]; 
        char leftover[ 10 ] = ""; // Mảng lưu tối đa 9 ký tự đuôi của lần nhận trước
        int total_count = 0;

        while (1) {
            int ret = recv(client, buf, sizeof(buf) - 1, 0);
            if (ret <= 0) break; // Client ngắt kết nối
            buf[ ret ] = '\0'; // Ngắt chuỗi để an toàn

            // Ghép phần đuôi của lần trước với gói dữ liệu mới
            strcpy(temp, leftover);
            strcat(temp, buf);

            // Tìm và đếm số lần xuất hiện của "0123456789"
            char *p = temp;
            while ((p = strstr(p, "0123456789")) != NULL) {
                total_count++;
                p += 10; // Bỏ qua 10 ký tự vừa tìm thấy để tìm tiếp
            }

            printf("Da cap nhat. So lan xuat hien hien tai la: %d\n", total_count);

            // Cắt giữ lại tối đa 9 ký tự cuối cùng của chuỗi temp cho lần sau
            int len = strlen(temp);
            if (len >= 9) {
                strcpy(leftover, temp + len - 9);
            } else {
                strcpy(leftover, temp);
            }
        }
        printf("Client da ngat ket noi. Tong ket co %d chuoi.\n", total_count);
        close(client);
    }
    close(listener);
    return 0;
}