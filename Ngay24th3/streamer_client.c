#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[ ]) {
    if (argc != 3) {
        printf("Cu phap: %s <IP> <Cổng>\n", argv[ 0 ]);
        return 1;
    }

    // Khoi tao ket noi
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[ 1 ]);
    addr.sin_port = htons(atoi(argv[ 2 ]));

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("Khong the ket noi den server!\n");
        return 1;
    }

    // Các đoạn dữ liệu giả lập bị vỡ vụn ngang qua chuỗi 0123456789 (Theo đúng ví dụ)
    char *chunks[ ] = {
        "SOICTSOICT012345678901234567890123456789012345",
        "6789SOICTSOICTSOICT012345678901234567890123456",
        "7890123456789012345678901234567890123456789012",
        "3456789SOICTSOICT01234567890123456789012345678",
        "9" // Ký tự cuối cùng của đoạn vắt ngang cuối
    };

    printf("Bat dau truyen du lieu streaming...\n");
    for (int i = 0; i < 5; i++) {
        send(client, chunks[ i ], strlen(chunks[ i ]), 0);
        printf("Da gui lan %d\n", i + 1);
        sleep(2); // Tam dung 2 giay de ban nhin thay ro server đếm dữ liệu theo thời gian thực
    }

    printf("Da gui xong. Ket thuc!\n");
    close(client);
    return 0;
}