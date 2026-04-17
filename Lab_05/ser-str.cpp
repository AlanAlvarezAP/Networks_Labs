#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct MyStruct {
    int id;
    float value;
};

int main() {

    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

    listen(server_fd, 5);

    printf("Server waiting...\n");

    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);

    struct MyStruct data;

    ssize_t total = 0;
    ssize_t n;

    while (total < sizeof(data)) {

        n = read(client_fd,
                 ((char*)&data) + total,
                 sizeof(data) - total);

        if (n <= 0) {
            printf("Connection closed or error\n");
            break;
        }

        total += n;
    }

    printf("Received struct:\n");
    printf("id = %d\n", data.id);
    printf("value = %f\n", data.value);

    close(client_fd);
    close(server_fd);

    return 0;
}