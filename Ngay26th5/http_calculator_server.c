#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    printf("HTTP Calculator Server dang chay tai http://127.0.0.1:8080/\n");
    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client == -1) continue;
        char buf[2048];
        int bytes = recv(client, buf, sizeof(buf) - 1, 0);
        if (bytes <= 0) {
            close(client);
            continue;
        }
        buf[bytes] = '\0';
        float a = 0, b = 0, result = 0;
        char cmd[16] = "";
        int has_calc = 0;
        char error_msg[256] = "";
        char *a_ptr = strstr(buf, "a=");
        char *b_ptr = strstr(buf, "b=");
        char *cmd_ptr = strstr(buf, "cmd=");
        if (a_ptr && b_ptr && cmd_ptr) {
            has_calc = 1;
            sscanf(a_ptr, "a=%f", &a);
            sscanf(b_ptr, "b=%f", &b);
            sscanf(cmd_ptr, "cmd=%15[^& \r\n]", cmd);
            if (strcmp(cmd, "add") == 0) result = a + b;
            else if (strcmp(cmd, "sub") == 0) result = a - b;
            else if (strcmp(cmd, "mul") == 0) result = a * b;
            else if (strcmp(cmd, "div") == 0) {
                if (b == 0) strcpy(error_msg, "Loi: Khong the chia cho 0!");
                else result = a / b;
            } else {
                strcpy(error_msg, "Phep toan khong hop le!");
            }
        }
        char html[2048];
        sprintf(html,
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<html><head><meta charset=\"UTF-8\"><title>Calculator</title></head><body>"
            "<h2>May Tinh HTTP (Ho tro GET & POST)</h2>"
            "<form>"
            "Toan hang A: <input type=\"number\" step=\"any\" name=\"a\" required> "
            "Phep toan: <select name=\"cmd\">"
            "<option value=\"add\">+</option>"
            "<option value=\"sub\">-</option>"
            "<option value=\"mul\">*</option>"
            "<option value=\"div\">/</option></select> "
            "Toan hang B: <input type=\"number\" step=\"any\" name=\"b\" required> <br><br>"
            "<input type=\"submit\" value=\"Tinh bang GET\" formmethod=\"GET\"> "
            "<input type=\"submit\" value=\"Tinh bang POST\" formmethod=\"POST\">"
            "</form><hr>"
        );
        if (has_calc) {
            if (strlen(error_msg) > 0) {
                sprintf(html + strlen(html), "<h3 style=\"color:red\">%s</h3>", error_msg);
            } else {
                char op_char = '?';
                if(strcmp(cmd, "add") == 0) op_char = '+';
                if(strcmp(cmd, "sub") == 0) op_char = '-';
                if(strcmp(cmd, "mul") == 0) op_char = '*';
                if(strcmp(cmd, "div") == 0) op_char = '/';
                sprintf(html + strlen(html), "<h3 style=\"color:blue\">Ket qua: %g %c %g = %g</h3>", a, op_char, b, result);
            }
        }
        strcat(html, "</body></html>");
        send(client, html, strlen(html), 0);
        close(client);
    }
    return 0;
}
