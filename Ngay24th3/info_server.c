#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Kiem tra tham so
    if (argc != 2) {
        printf("Cu phap: %s <Cổng>\n", argv[ 0 ]);
        return 1;
    }

    // Tao socket server
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[ 1 ])); // Lay cong tu argv[ 1 ]

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    printf("Server dang cho ket noi o cong %s...\n", argv[ 1 ]);

    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;

        char buf[ 4096 ]; // Mang bo dem 4096 bytes
        
        // Nhan goi du lieu
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret > 0) {
            int offset = 0;
            
            // Boc tach ten thu muc dau tien
            printf("%s\n", buf + offset);
            offset += strlen(buf + offset) + 1;

            // Boc tach tuan tu ten file va kich thuoc
            while (offset < ret) {
                char *filename = buf + offset;
                offset += strlen(filename) + 1;

                long size;
                memcpy(&size, buf + offset, sizeof(long));
                offset += sizeof(long);

                printf("%s - %ld bytes\n", filename, size);
            }
        }
        close(client);
    }
    close(listener);
    return 0;
}