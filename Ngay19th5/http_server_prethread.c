#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

void* worker_thread(void *arg) {
    int listener = *(int*)arg;
    char buf[2048];
    while(1){
        int client = accept(listener, NULL, NULL);
        int ret = recv(client, buf, sizeof(buf) - 1, 0);
        if (ret > 0) {
            char *html = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao! Toi la Prethreading Server.</h1></body></html>";
            send(client, html, strlen(html), 0);
            printf(">> Luong (Thread ID: %ld) vua phuc vu 1 yeu cau.\n", pthread_self());
        }
        close(client);
    }
    return NULL;
}
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);
    printf("HTTP Prethreading chay tai http://127.0.0.1:8080/\n");
    int num_threads = 8;
    pthread_t thread_id;
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&thread_id, NULL, worker_thread, &listener);
        pthread_detach(thread_id); 
    }
    while(1) { sleep(100); }
    return 0;
}