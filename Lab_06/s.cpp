#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 45000
#define BUFFER_SIZE 5


bool error_handler(int bytes, int &tries){
    if (bytes > 0) {
        return false;   
    }
    else if (bytes == 0) {
        std::cout << "Client close the connection (EOF)\n";
        return true;
    }
    else {
        if (errno == EINTR) {
            if(tries>=3){
                return true;
            }
            tries++;
            return false;
        }

        if (errno == ECONNRESET) {
            std::cout << "Client se desconectó abruptamente\n";
        } else {
            std::cerr << "Error en recv: " << strerror(errno) << std::endl;
        }
        return true;
    }
}

bool is_right(const std::string &ident,const std::string &json_fil){
    if(ident != "J"){
        std::cout << "Wrong identifier" << std::endl;
        return false;
    }
    if(json_fil[0] != '{'){
        std::cout << "Wrong format in Json {" << std::endl;
        return false;        
    }
    if(json_fil[json_fil.size()-1] != '}'){
        std::cout << "Wrong format in Json }" << std::endl;
        return false;
    }
    return true;
}

void read_fragment(int client_socket,int &tries,int size_to_read,std::string& msg,bool &check_errors){
    char buffer[99999];
    bzero(buffer,99999);
    int tama=size_to_read;
    while(tama > 0){
        int n=recv(client_socket,buffer,tama,0);
        if(error_handler(n,tries)){
            check_errors=true;
            return;
        }
        buffer[n]='\0';
        msg+=buffer;
        tama-=n;
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string acumulator;
    int tries=0;
    bool error=false;
    while (true) {
        std::string size,identifier,json_fil;

        size.clear();
        identifier.clear();
        json_fil.clear();

        usleep(200000);
        read_fragment(client_socket,tries,5,size,error);
        if(error) break;

        read_fragment(client_socket,tries,1,identifier,error);
        if(error) break;

        read_fragment(client_socket,tries,std::stoi(size),json_fil,error);
        if(error) break;

        if(is_right(identifier,json_fil)){
            std::cout << "Data arrived ->" << "size = " << size << " ; identifier = " << identifier << " ; json fil = " << json_fil << std::endl; 
        }
        
    }

    close(client_socket);
    std::cout << "Conexión cerrada\n";
}

int main() {
    // Evitar que el proceso muera por SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Reutilizar puerto
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "Servidor escuchando en puerto " << PORT << std::endl;

    sockaddr_in client_addr{};
    socklen_t addrlen = sizeof(client_addr);

    int client_socket = accept(server_fd,
                                (sockaddr*)&client_addr,
                                &addrlen);

    if (client_socket < 0) {
        perror("accept");
    }

    std::cout << "Cliente conectado\n";
    handle_client(client_socket);

    close(server_fd);
    return 0;
}