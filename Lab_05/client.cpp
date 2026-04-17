#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct MyStruct {
    int id;
    float value;
};

int main() {

    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    struct MyStruct data = {1, 3.14f};

    write(sock, (char*)&data, sizeof(data));

    printf("Struct sent\n");

    close(sock);

    return 0;
}
