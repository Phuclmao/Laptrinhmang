#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

void send_cmd(int sock, char *cmd) {
    send(sock, cmd, strlen(cmd), 0);
    printf(">> CLIENT: %s", cmd);
}

int recv_res(int sock, char *buf, int size) {
    int bytes = recv(sock, buf, size - 1, 0);
    if (bytes > 0) {
        buf[bytes] = '\0';
        printf(">> SERVER: %s", buf);
    }
    return bytes;
}

int enter_pasv(int control_sock) {
    char buf[1024];
    send_cmd(control_sock, "PASV\r\n");
    recv_res(control_sock, buf, sizeof(buf));
    int ip1, ip2, ip3, ip4, p1, p2;
    char *start = strchr(buf, '(');
    if (start) {
        sscanf(start, "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
        char ip_str[64];
        sprintf(ip_str, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
        int port = p1 * 256 + p2;
        int data_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr(ip_str);
        data_addr.sin_port = htons(port);
        if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) == 0) {
            printf("[*] Da mo kenh Du lieu (Data Connection) thanh cong!\n");
            return data_sock;
        }
    }
    return -1;
}

void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}
int main() {
    char buf[1024];
    char *mssv = "20235398";
    char *mat_khau = "539802";

    // Phân giải tên miền lebavui.io.vn
    struct addrinfo *res;
    if (getaddrinfo("lebavui.io.vn", "21", NULL, &res) != 0) {
        printf("Loi phan giai ten mien!\n");
        return 1;
    }

    int control_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(control_sock, res->ai_addr, res->ai_addrlen);
    recv_res(control_sock, buf, sizeof(buf));

    char cmd[512];
    sprintf(cmd, "USER user_%s\r\n", mssv);
    send_cmd(control_sock, cmd);
    recv_res(control_sock, buf, sizeof(buf));

    sprintf(cmd, "PASS %s\r\n", mat_khau);
    send_cmd(control_sock, cmd);
    recv_res(control_sock, buf, sizeof(buf));

    int data_sock = enter_pasv(control_sock);
    send_cmd(control_sock, "LIST\r\n");
    recv_res(control_sock, buf, sizeof(buf));

    char list_buf[4096] = "";
    int bytes = recv(data_sock, list_buf, sizeof(list_buf) - 1, 0);
    if (bytes > 0) list_buf[bytes] = '\0';
    close(data_sock);
    recv_res(control_sock, buf, sizeof(buf));

    char q_filename[256] = "";
    char a_filename[256] = "";
    char *p = strstr(list_buf, "question_");
    if (p) {
        sscanf(p, "%s", q_filename);
        printf("\n[*] Tim thay file: %s\n", q_filename);
        strcpy(a_filename, q_filename);
        strncpy(a_filename, "answer", 6); 
    } else {
        printf("Khong tim thay file question tren server!\n");
        return 1;
    }

    data_sock = enter_pasv(control_sock);
    sprintf(cmd, "RETR %s\r\n", q_filename);
    send_cmd(control_sock, cmd);
    recv_res(control_sock, buf, sizeof(buf));

    char file_content[4096] = "";
    bytes = recv(data_sock, file_content, sizeof(file_content) - 1, 0);
    if (bytes > 0) file_content[bytes] = '\0';
    close(data_sock);
    recv_res(control_sock, buf, sizeof(buf));
    file_content[strcspn(file_content, "\r\n")] = 0;
    printf("\n[*] Noi dung file goc: %s\n", file_content);

    reverse_string(file_content);
    printf("[*] Noi dung dao nguoc: %s\n\n", file_content);
    
    FILE *f = fopen(a_filename, "wb");
    fprintf(f, "%s", file_content);
    fclose(f);
    printf("[*] Da luu vao file local: %s\n", a_filename);

    data_sock = enter_pasv(control_sock);
    sprintf(cmd, "STOR %s\r\n", a_filename);
    send_cmd(control_sock, cmd);
    recv_res(control_sock, buf, sizeof(buf));

    f = fopen(a_filename, "rb");
    while ((bytes = fread(buf, 1, sizeof(buf), f)) > 0) {
        send(data_sock, buf, bytes, 0);
    }
    fclose(f);
    close(data_sock);
    recv_res(control_sock, buf, sizeof(buf));

    send_cmd(control_sock, "QUIT\r\n");
    recv_res(control_sock, buf, sizeof(buf));
    close(control_sock);

    printf("\n[*] HOAN THANH BAI TAP FTP!\n");
    return 0;
}