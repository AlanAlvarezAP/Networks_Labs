/* Client code in C */ 
#include "DataStructure.hpp"

Client_Protocols clp;

void print_menu() {
    std::cout << "===================================" << std::endl;
    std::cout << "|          Welcome to             |" << std::endl;
    std::cout << "|        Chat Simulation          |" << std::endl;
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

char Cast_Option(int option){
	switch(option){
        	case 1: {
            		return 'L';
        	}
		case 2:{
			return 'O';
		}
		case 3:{
			return 'B';
		}
		case 4:{
			return 'b';
		}
		default:{
			return 'z';
		}
	}
}

void read_thread(int n,int SocketFD){
    char buffer;
    while (clp.running) {
        n = read(SocketFD, &buffer, 1);
        if (n <= 0) {
            std::cout << "Disconection from server closing..." << std::endl;
            close(SocketFD);
	    clp.running=false;
	    clp.logging_status=false;
            break;
        }
        clp.Cases_Client(buffer, n, SocketFD);
    }
     
}

int main(void){
    while(true){
	struct sockaddr_in stSockAddr;
    	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    	int n;

    	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    	stSockAddr.sin_family = AF_INET;
    	stSockAddr.sin_port = htons(45000);
    	inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    	connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

	std::cout << "CREANDO NUEVO CLIENTE CON SOCKET " << SocketFD << std::endl;

	clp.logging_status=false; 
	clp.running=true;
    	print_menu();

    	std::thread(read_thread,n,SocketFD).detach();
    	for(;;) {
        	std::cout << "SELECT AN ACTION :D " << std::endl;
        	int action;
        	std::cin >> action;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        	char option = Cast_Option(action);
        	if (option != 'L' && clp.logging_status == false) {
            		std::cout << "You are not logged in, try logging pls :D" << std::endl;
            		print_menu();
            		continue;
        	}
        	clp.Cases_Client(option, n, SocketFD);

		if(option == 'O'){
			clp.logging_status=false;
			clp.running=false;
		}

		if(option != 'L' && (!clp.logging_status || !clp.running)){
			break;
		}
    	}
    }
    
    return 0;
}
