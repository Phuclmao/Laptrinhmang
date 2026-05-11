#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    printf("HTTP Preforking Server dang chay tai http://127.0.0.1:8080/\n");
    int num_processes = 8;
    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) {
            char buf[2048];
            while(1){
                int client = accept(listener, NULL, NULL);
                int ret = recv(client, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    close(client);
                    continue;
                }
                char *html_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban. Toi la HTTP Prefork Server!</h1></body></html>";
                send(client, html_response, strlen(html_response), 0);
                printf(">> Tien trinh PID %d vua xu ly 1 request.\n", getpid());
                close(client);
            }
        }
    }
    wait(NULL);
    return 0;
}