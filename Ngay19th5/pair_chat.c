#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>

struct Pair {
    int client1;
    int client2;
};

void* pair_chat_handler(void* arg) {
    struct Pair *p = (struct Pair*)arg;
    struct pollfd fds[2];
    char buf[1024];
    fds[0].fd = p->client1; fds[0].events = POLLIN;
    fds[1].fd = p->client2; fds[1].events = POLLIN;
    char *msg1 = "Da ghep cap! Ban co the bat dau chat.\n";
    send(p->client1, msg1, strlen(msg1), 0);
    send(p->client2, msg1, strlen(msg1), 0);
    while(1){
        int ret = poll(fds, 2, -1);
        if(ret < 0) break;
        if(fds[0].revents & (POLLIN | POLLERR)){
            int bytes = recv(p->client1, buf, sizeof(buf)-1, 0);
            if (bytes <= 0) break;
            buf[bytes] = '\0';
            send(p->client2, buf, bytes, 0);
        }
        if(fds[1].revents & (POLLIN | POLLERR)) {
            int bytes = recv(p->client2, buf, sizeof(buf)-1, 0);
            if (bytes <= 0) break;
            buf[bytes] = '\0';
            send(p->client1, buf, bytes, 0);
        }
    }
    printf(">> Mot cap chat da ngat ket noi.\n");
    close(p->client1);
    close(p->client2);
    free(p);
    return NULL;
}
int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);
    printf("Pair Chat Server dang lang nghe o cong 9000...\n");
    while(1){
        printf("Dang doi Client 1...\n");
        int c1 = accept(listener, NULL, NULL);
        send(c1, "Dang cho nguoi ghep cap...\n", 27, 0);
        printf("Dang doi Client 2...\n");
        int c2 = accept(listener, NULL, NULL);
        struct Pair *p = malloc(sizeof(struct Pair));
        p->client1 = c1;
        p->client2 = c2;
        pthread_t tid;
        pthread_create(&tid, NULL, pair_chat_handler, (void*)p);
        pthread_detach(tid);
    }
    return 0;
}