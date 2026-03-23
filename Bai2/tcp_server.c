#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <Cổng> <Tệp tin chứa câu chào> <Tệp tin lưu nội dung client gửi đến>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int opt = 1; 
    // Thiết lập SO_REUSEADDR để giải phóng cổng ngay khi server tắt 
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }

    int ret = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind() failed");
        exit(1);
    }

    ret = listen(listener, 5);
    if (ret < 0) {
        perror("listen() failed");
        exit(1);
    }

    printf("Waiting for client\n");
    
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client < 0) {
        perror("accept() failed");
        exit(1);
    }

    printf("Client connected: %d\n", client);
    printf("IP: %s\n", inet_ntoa(client_addr.sin_addr));
    printf("Port: %d\n", ntohs(client_addr.sin_port));

    // Đọc tệp tin chứa câu chào
    char hello_path[] = "/mnt/c/Users/ACER/test/";
    strcat(hello_path, argv[2]);
    FILE *greeting_file = fopen(hello_path, "r");
    if (greeting_file == NULL) {
        printf("Failed to open greeting file %s", argv[2]);
        return 1;
    }
    
    char greeting[1024];
    if (fgets(greeting, sizeof(greeting), greeting_file) == NULL) {
        perror("Failed to read greeting from file");
        fclose(greeting_file);
        return 1;
    }
    fclose(greeting_file);

    // Mở tệp tin lưu nội dung client gửi đến
    char write_path[] = "/mnt/c/Users/ACER/test/";
    strcat(write_path, argv[3]);
    FILE *log_file = fopen(write_path, "w");
    if (log_file == NULL) {
        printf("Failed to open log file %s", argv[3]);
        return 1;
    }

    // Gửi câu chào đến client
    int s = send(client, greeting, strlen(greeting), 0);
    if (s < 0) {
        perror("send() failed");
        close(client);
        close(listener);
        return 1;
    }

    char buf[256];

    while (1) {
        int n = recv(client, buf, sizeof(buf), 0);
        
        if (n <= 0) {
            printf("Disconnected\n");
            break;
        }

        buf[n] = 0; // Them ky tu ket thuc xau
        fputs(buf, log_file);
        printf("%d bytes received: %s\n", n, buf);
    }

    fclose(log_file);

    close(client);
    close(listener);
}