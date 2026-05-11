#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

void signalHandler(int signo) {
    wait(NULL);
}

int check_login(char *user, char *pass) {
    FILE *f = fopen("database.txt", "r");
    if (!f) return 0; 
    char f_user[64], f_pass[64];
    while (fscanf(f, "%s %s", f_user, f_pass) == 2) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f); return 1; 
        }
    }
    fclose(f);
    return 0;
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
    printf("Telnet Multiprocess Server dang chay o cong 9000...\n");
    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;
        printf(">> Co client moi ket noi!\n");
        if (fork() == 0) {
            close(listener);
            char *msg = "Vui long nhap tai khoan (user pass):\n";
            send(client, msg, strlen(msg), 0);
            char buf[1024];
            int logged_in = 0;
            while (1) {
                int bytes = recv(client, buf, sizeof(buf) - 1, 0);
                if (bytes <= 0) break;
                buf[bytes] = '\0';
                buf[strcspn(buf, "\r\n")] = 0;
                if (logged_in == 0) {
                    char user[64], pass[64];
                    if (sscanf(buf, "%s %s", user, pass) == 2 && check_login(user, pass)) {
                        logged_in = 1;
                        char *ok = "Dang nhap thanh cong! Ban co the nhap lenh:\n";
                        send(client, ok, strlen(ok), 0);
                    } else {
                        char *err = "Tai khoan hoac mat khau sai!\n";
                        send(client, err, strlen(err), 0);
                    }
                } else {
                    char cmd[1100];
                    sprintf(cmd, "%s > out.txt", buf);
                    system(cmd);
                    FILE *f = fopen("out.txt", "r");
                    if (f) {
                        char out_buf[1024];
                        int has_out = 0;
                        while (fgets(out_buf, sizeof(out_buf), f)) {
                            send(client, out_buf, strlen(out_buf), 0);
                            has_out = 1;
                        }
                        if (!has_out) send(client, "(Khong co ket qua)\n", 19, 0);
                        fclose(f);
                    }
                }
            }
            printf(">> Client da thoat. Dong tien trinh con.\n");
            close(client);
            exit(0);
        }
        close(client);
    }
    return 0;
}