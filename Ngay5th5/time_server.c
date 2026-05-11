#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
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
    printf("Time Server Multiprocessing dang chay o cong 9000...\n");
    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;
        if (fork() == 0) {
            close(listener);
            char buf[256];
            char *welcome = "Vui long nhap lenh (VD: GET_TIME dd/mm/yyyy):\n";
            send(client, welcome, strlen(welcome), 0);
            while(1){
                int bytes = recv(client, buf, sizeof(buf) - 1, 0);
                if (bytes <= 0) break;
                buf[bytes] = '\0';
                buf[strcspn(buf, "\r\n")] = 0;
                char cmd[32] = "", format[32] = "";
                int parsed = sscanf(buf, "%s %s", cmd, format);
                if (parsed == 2 && strcmp(cmd, "GET_TIME") == 0) {
                    time_t t = time(NULL);
                    struct tm *tm_info = localtime(&t);
                    char time_str[64] = "";
                    int valid = 1;
                    if (strcmp(format, "dd/mm/yyyy") == 0) {
                        strftime(time_str, sizeof(time_str), "%d/%m/%Y\n", tm_info);
                    } else if (strcmp(format, "dd/mm/yy") == 0) {
                        strftime(time_str, sizeof(time_str), "%d/%m/%y\n", tm_info);
                    } else if (strcmp(format, "mm/dd/yyyy") == 0) {
                        strftime(time_str, sizeof(time_str), "%m/%d/%Y\n", tm_info);
                    } else if (strcmp(format, "mm/dd/yy") == 0) {
                        strftime(time_str, sizeof(time_str), "%m/%d/%y\n", tm_info);
                    } else {
                        valid = 0;
                    }
                    if (valid) {
                        send(client, time_str, strlen(time_str), 0);
                    } else {
                        char *err = "Format khong ho tro. (Dung: dd/mm/yyyy, dd/mm/yy, mm/dd/yyyy, mm/dd/yy)\n";
                        send(client, err, strlen(err), 0);
                    }
                } else {
                    char *err2 = "Sai cu phap. Vui long dung lenh: GET_TIME [format]\n";
                    send(client, err2, strlen(err2), 0);
                }
            }
            close(client);
            exit(0);
        }
        close(client);
    }
    return 0;
}