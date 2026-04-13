/* Server code in C */
#include "DataStructure.hpp"

std::unordered_map<std::string,int> little_map;
Protocols_Receivers rcv;
Protocols_Senders snd;

void read_thread(int n,int SocketFD){
    char buffer;
    for(;;){
        n = read(SocketFD, &buffer, 1);
        rcv.Receive_Protocol(buffer, n, SocketFD);
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
      rcv.Receive_Protocol('L', n, ClientFD,&little_map);

      std::thread (read_thread,n,ClientFD).detach();
    }
 
    shutdown(ClientFD, SHUT_RDWR);
 
    close(ClientFD);
 
    close(ServerFD);
    return 0;
}
