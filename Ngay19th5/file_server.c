#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

#define SHARED_DIR "./shared_folder"

void signalHandler(int signo) {
    wait(NULL);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    signal(SIGCHLD, signalHandler);
    printf("File Server dang chay o cong 9000...\n");
    while(1){
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;
        if (fork() == 0) {
            close(listener);
            DIR *d = opendir(SHARED_DIR);
            struct dirent *dir;
            char file_list[2048] = "";
            int file_count = 0;
            if(d){
                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_type == DT_REG) {
                        strcat(file_list, dir->d_name);
                        strcat(file_list, "\r\n");
                        file_count++;
                    }
                }
                closedir(d);
            }
            if (file_count == 0) {
                char *err = "ERROR No files to download\r\n";
                send(client, err, strlen(err), 0);
                close(client);
                exit(0);
            } else {
                char header[64];
                sprintf(header, "OK %d\r\n", file_count);
                send(client, header, strlen(header), 0);
                send(client, file_list, strlen(file_list), 0);
                send(client, "\r\n", 2, 0);
            }
            char buf[256];
            while(1){
                int bytes = recv(client, buf, sizeof(buf) - 1, 0);
                if (bytes <= 0) break;
                buf[bytes] = '\0';
                buf[strcspn(buf, "\r\n")] = 0;
                char filepath[512];
                sprintf(filepath, "%s/%s", SHARED_DIR, buf);
                FILE *f = fopen(filepath, "rb");
                if(f){
                    struct stat st;
                    stat(filepath, &st);
                    char res_header[64];
                    sprintf(res_header, "OK %ld\r\n", st.st_size);
                    send(client, res_header, strlen(res_header), 0);
                    char file_data[1024];
                    int read_bytes;
                    while((read_bytes = fread(file_data, 1, sizeof(file_data), f)) > 0){
                        send(client, file_data, read_bytes, 0);
                    }
                    fclose(f);
                    break; 
                } else {
                    char *err = "ERROR File not found\r\nVui long nhap lai ten file: ";
                    send(client, err, strlen(err), 0);
                }
            }
            close(client);
            exit(0);
        }
        close(client);
    }
    return 0;
}