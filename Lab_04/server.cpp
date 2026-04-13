/* Server code in C */
#include "DataStructure.hpp"

Server_Protocols sv;

void read_thread(int n,int SocketFD){
    std::cout << "THREAD INICIADO CON SocketFD " << SocketFD << std::endl;
    char buffer;
    for(;;){
        n = read(SocketFD, &buffer, 1);
        if (n <= 0) {
            std::cout << "Disconection from client closing..." << std::endl;
            sv.Cases_Server('O', n, SocketFD);
            break;
        }
        sv.Cases_Server(buffer, n, SocketFD);
    }
    
}

int main(void){
    struct sockaddr_in stSockAddr;
    int ServerFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    bind(ServerFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));
 
    listen(ServerFD, 10);
    
    int ClientFD=0;
    for(;;){
        ClientFD=accept(ServerFD,NULL,NULL);
	std::cout << "ACCEPT - ClientFD: " << ClientFD << "with port " << stSockAddr.sin_port << std::endl;
        std::thread (read_thread,n,ClientFD).detach();
    }
 
    shutdown(ClientFD, SHUT_RDWR);
 
    close(ClientFD);
 
    close(ServerFD);
    return 0;
}
