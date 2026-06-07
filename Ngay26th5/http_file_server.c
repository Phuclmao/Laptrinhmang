#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>

void signalHandler(int signo) {
    wait(NULL);
}
void url_decode(char *src, char *dest) {
    while(*src){
        if (*src == '%' && src[1] && src[2]) {
            int hex;
            sscanf(src + 1, "%2x", &hex);
            *dest++ = hex;
            src += 3;
        } else if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}
const char* get_content_type(const char* path) {
    const char *dot = strrchr(path, '.');
    if (!dot) return "application/octet-stream";
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".txt") == 0 || strcmp(dot, ".c") == 0) return "text/plain";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".mp3") == 0) return "audio/mpeg";
    if (strcmp(dot, ".mp4") == 0) return "video/mp4";
    return "application/octet-stream";
}
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);
    signal(SIGCHLD, signalHandler);
    printf("HTTP File Server dang chay tai http://127.0.0.1:8080/\n");
    while(1){
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;
        if (fork() == 0) {
            close(listener);
            char buf[4096];
            int bytes = recv(client, buf, sizeof(buf) - 1, 0);
            if (bytes > 0) {
                buf[bytes] = '\0';
                char method[16], req_path[1024], protocol[16];
                sscanf(buf, "%s %s %s", method, req_path, protocol);
                char decoded_path[1024];
                url_decode(req_path, decoded_path);
                char local_path[2048];
                sprintf(local_path, ".%s", decoded_path); 
                struct stat st;
                if (stat(local_path, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
                                       "<html><head><title>File Server</title></head><body>"
                                       "<h2>Danh sach thu muc: %s</h2><ul>";
                        char res_buf[2048];
                        sprintf(res_buf, header, decoded_path);
                        send(client, res_buf, strlen(res_buf), 0);
                        DIR *d = opendir(local_path);
                        struct dirent *dir;
                        if(d){
                            while ((dir = readdir(d)) != NULL) {
                                if (strcmp(dir->d_name, ".") == 0 || dir->d_name[0] == '.') continue;
                                char child_path[4096];
                                snprintf(child_path, sizeof(child_path), "%s/%s", local_path, dir->d_name);
                                struct stat child_st;
                                stat(child_path, &child_st);
                                char link_path[4096];
                                if (strcmp(decoded_path, "/") == 0) 
                                    sprintf(link_path, "/%s", dir->d_name);
                                else 
                                    sprintf(link_path, "%s/%s", decoded_path, dir->d_name);
                                char item_html[8192];
                                if (S_ISDIR(child_st.st_mode)) {
                                    snprintf(item_html, sizeof(item_html), "<li><a href=\"%s\"><b>%s/</b></a></li>", link_path, dir->d_name);
                                } else {
                                    snprintf(item_html, sizeof(item_html), "<li><a href=\"%s\"><i>%s</i></a></li>", link_path, dir->d_name);
                                }
                                send(client, item_html, strlen(item_html), 0);
                            }
                            closedir(d);
                        }
                        char *footer = "</ul></body></html>";
                        send(client, footer, strlen(footer), 0);
                    } else {
                        const char *ctype = get_content_type(local_path);
                        FILE *f = fopen(local_path, "rb");
                        if (f) {
                            char header[1024];
                            sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", ctype, st.st_size);
                            send(client, header, strlen(header), 0);
                            char fbuf[4096];
                            int read_bytes;
                            while ((read_bytes = fread(fbuf, 1, sizeof(fbuf), f)) > 0) {
                                send(client, fbuf, read_bytes, 0);
                            }
                            fclose(f);
                        }
                    }
                } else {
                    char *err = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 File Not Found</h1>";
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