/* Client code in C */
 
#include "DataStructure.hpp"
Protocols_Receivers rcv;
Protocols_Senders snd;
 
void print_menu() {
    std::cout << "===================================" << std::endl;
    std::cout << "|          Bienvenido a           |" << std::endl;
    std::cout << "|        Simulador de Chat        |" << std::endl;
    std::cout << "|                                 |" << std::endl;
    std::cout << "|  1. Login                       |" << std::endl;
    std::cout << "|  2. Logout                      |" << std::endl;
    std::cout << "|  3. Broadcast                   |" << std::endl;
    std::cout << "|  4. Unicast                     |" << std::endl;
    /*std::cout << "|  f. Rotar inverso (0.1)         |" << std::endl;
    std::cout << "|  g. Escalar (1.1)               |" << std::endl;
    std::cout << "|  h. Escalar inverso (0.9)       |" << std::endl;
    std::cout << "|  x. Llenar pedidos Scale In-Out |" << std::endl;
    std::cout << "|  z. Animacion extra (all animation sin escala) con rebanada|" << std::endl;
    std::cout << "|  v. Animacion extra (all animation sin escala) todos|" << std::endl;
    std::cout << "|  4. Mover arriba (Flecha arr)   |" << std::endl;
    std::cout << "|  5. Mover abajo (Flecha abj)    |" << std::endl;
    std::cout << "|  6. Mover derecha (Flecha der)  |" << std::endl;
    std::cout << "|  7. Mover izquierda (Flecha izq)|" << std::endl;
    std::cout << "|  8. Salir (ESC o CTRL+C)        |" << std::endl;*/
    std::cout << "===================================" << std::endl;
}

void read_thread(int n,int SocketFD){
    char buffer;
    for (;;) {
        n = read(SocketFD, buffer, 1);
        rcv.Receive_Protocol(buffer, n, SocketFD);
    }
     
}

int main(void){
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

    snd.Send_Protocol('L', n, SocketFD);

    std::thread(read_thread,n,SocketFD).detach();
    for(;;){
        print_menu();
        std::cout << "SELECT AN ACTION :D " << std::endl;
        std::string action;
        std::getline(std::cin,action);
        if(msg == "exit"){
            break;
        }
        snd.Send_Protocol(action[0], n, SocketFD);
    }
   
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
}
