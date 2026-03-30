#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Cu phap: %s <IP_Server> <Cổng>\n", argv[ 0 ]);
        return 1;
    }

    // Tao socket va ket noi
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[ 1 ]); // Lay IP tu argv[ 1 ]
    addr.sin_port = htons(atoi(argv[ 2 ]));      // Lay cong tu argv[ 2 ]

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("Khong the ket noi den server!\n");
        return 1;
    }

    char buf[ 4096 ]; // Mang bo dem 4096 bytes
    int offset = 0; 

    // 1. Lay ten thu muc hien tai
    char cwd[ 1024 ]; // Mang 1024 bytes
    getcwd(cwd, sizeof(cwd));
    strcpy(buf + offset, cwd);
    offset += strlen(cwd) + 1; 

    // 2. Doc thu muc va lay ten, kich thuoc file
    DIR *d;
    struct dirent *dir;
    struct stat st;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                stat(dir->d_name, &st);
                
                strcpy(buf + offset, dir->d_name);
                offset += strlen(dir->d_name) + 1;
                
                long size = st.st_size;
                memcpy(buf + offset, &size, sizeof(long));
                offset += sizeof(long);
            }
        }
        closedir(d);
    }

    // 3. Gui du lieu
    send(client, buf, offset, 0);
    printf("Da gui %d bytes du lieu sang server.\n", offset);

    close(client);
    return 0;
}
