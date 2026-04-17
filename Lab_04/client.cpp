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
    std::cout << "|  5. Receive all clients         |" << std::endl;
    std::cout << "|  6. Send file                   |" << std::endl;
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
			return 'U';
		}
        case 5:{
            return 'T';
        }
        case 6:{
            return 'F';
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
        if(n == 0){
            std::cout << "Server closed connection because of logout :D " << std::endl;
            close(SocketFD);
	        clp.running=false;
	        clp.logging_status=false;
            break;
        }
        else if (n < 0) {
            std::cout << "Disconection from server because of an ERROR..." << std::endl;
            close(SocketFD);
	        clp.running=false;
	        clp.logging_status=false;
            break;
        }
        clp.Cases_Client(buffer, n, SocketFD);
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

    clp.running = true;
    print_menu();

    std::thread(read_thread,n,SocketFD).detach();
    while(clp.running) {
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

    }
    std::cout << " LEAVING ... " << std::endl;
    return 0;
}
