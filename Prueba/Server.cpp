#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <map>
#include <mutex>

/*
=================================
  Variables Globales de Control
=================================
*/

std::map <std::string, int> Registro_Clientes; 
std::map<int, std::string> Nicknames_Clientes;

std::mutex Mutex_Clientes;

/*
=================================
      Funciones Auxiliares
=================================
*/

std::string Num_to_Str(int n, int Max_Size){

    std::string s = std::to_string(n);

    while(s.length() < Max_Size){

        s = "0" + s;

    }

    return s;

}

/*
=================================
    Funciones del Servidor
=================================
*/

void Login_Response(int Client_Socket){

    char Buffer_Size[5];

    read(Client_Socket, Buffer_Size, 4);

    Buffer_Size[4] = '\0'; 
    int NickName_Size = atoi(Buffer_Size);

    std::string NickName;
    NickName.resize(NickName_Size);
    
    read(Client_Socket, &NickName[0], NickName_Size);

    {

    std::lock_guard<std::mutex> lock(Mutex_Clientes);

    if(Registro_Clientes.find(NickName) != Registro_Clientes.end()){

        std::string Error = "Nickname ya en uso.";
        std::string Packet_Error = "E" + Num_to_Str(Error.length(), 5) + Error;
        write(Client_Socket, Packet_Error.c_str(), Packet_Error.length());
        return;

    }

    Registro_Clientes[NickName] = Client_Socket;
    Nicknames_Clientes[Client_Socket] = NickName;

    }

    write(Client_Socket, "K", 1);

}

void Logout_Response(int Client_Socket){

    std::string NickName;
    
    {

    std::lock_guard<std::mutex> lock(Mutex_Clientes);

    auto Iterador = Nicknames_Clientes.find(Client_Socket);

    if( Iterador == Nicknames_Clientes.end()){

        std::cout << "Socket no registrado.\n";
        return;

    }

    NickName = Iterador->second;

    Nicknames_Clientes.erase(Iterador);
    Registro_Clientes.erase(NickName);

    std::cout << "[Accion]: {" << NickName << "} desconectado.\n";

    }

    write(Client_Socket, "K", 1);

    close(Client_Socket);

}

void Unicast_Response(int Client_Socket){

    char Buffer_Size[8];

    read(Client_Socket, Buffer_Size, 5);

    Buffer_Size[5] = '\0';
    int Msg_Size = atoi(Buffer_Size);

    std::string Msg;
    Msg.resize(Msg_Size);

    read(Client_Socket, &Msg[0], Msg_Size);

    read(Client_Socket, Buffer_Size, 7);

    Buffer_Size[7] = '\0';
    int Nickname_Destino_Size = atoi(Buffer_Size);
    
    std::string Nickname_Destino;
    Nickname_Destino.resize(Nickname_Destino_Size);

    read(Client_Socket, &Nickname_Destino[0], Nickname_Destino_Size);

    int Socket_Destino;
    std::string Nickname_Origen;

    {

        std::lock_guard<std::mutex> lock(Mutex_Clientes);

        if(Registro_Clientes.find(Nickname_Destino) == Registro_Clientes.end()){

                std::string Error = "No existe este nickname [" + Nickname_Destino + "] en el servidor.";
                
                std::string Packet_Error = "E" + Num_to_Str(Error.length(), 5) + Error;

                write(Client_Socket, Packet_Error.c_str(), Packet_Error.length());

                std::cout << Error << "\n";

                return;

        }

        Socket_Destino = Registro_Clientes[Nickname_Destino];

        Nickname_Origen = Nicknames_Clientes[Client_Socket];

    }

    std::string Packet = "u" + Num_to_Str(Nickname_Origen.length(), 7) + Nickname_Origen
                       + Num_to_Str(Msg_Size, 5) + Msg;

    if(write(Socket_Destino, Packet.c_str(), Packet.length()) < 0){

        std::string Error = "No se pudo enviar el mensaje.";
            
        std::string Packet_Error = "E" + Num_to_Str(Error.length(), 5) + Error;

        write(Client_Socket, Packet_Error.c_str(), Packet_Error.length());

        std::cout << Error << "\n";

        return;

    }

    std::cout << "[" << Nickname_Origen << "]: envia:\n" << Packet << "\nPara [" <<  Nickname_Destino << "]\n";

}

void Broadcast_Response(int Client_Socket){
    char Buffer_Size[8];

    read(Client_Socket, Buffer_Size, 7);

    Buffer_Size[7] = '\0';
    int Msg_Size = atoi(Buffer_Size);

    std::string Msg;
    Msg.resize(Msg_Size);

    read(Client_Socket, &Msg[0], Msg_Size);

    std::string Nickname_Origen = Nicknames_Clientes[Client_Socket];

    std::string Packet = "b" + Num_to_Str(Nickname_Origen.length(), 3) + Nickname_Origen
                    + Num_to_Str(Msg_Size, 7) + Msg;

    for(auto Par : Registro_Clientes){

        if(write(Par.second, Packet.c_str(), Packet.length()) < 0){

        std::string Error = "No se pudo enviar el mensaje.";
            
        std::string Packet_Error = "E" + Num_to_Str(Error.length(), 5) + Error;

        write(Client_Socket, Packet_Error.c_str(), Packet_Error.length());

        std::cout << Error << "\n";

        return;

        }

        std::cout << "[" << Nickname_Origen << "]: envia:\n" << Packet << "\nPara [" <<  Par.first << "]\n";

    }

}

void List_Query_Response(int Client_Socket){}

void File_Response(int Client_Socket){}


void Leer_Redirigir(int Client_Socket){

    while(true){

        char Protocolo;

        int n = read(Client_Socket, &Protocolo, 1);

        if(n <= 0){

            std::cout << "Cliente desconectado.\n";
            Logout_Response(Client_Socket);
            break;

        }

        switch (Protocolo){

            case 'L':
                Login_Response(Client_Socket);
                break;

            case 'O':
                Logout_Response(Client_Socket);
                break; 

            case 'U':
                Unicast_Response(Client_Socket);
                break; 

            case 'B':
                Broadcast_Response(Client_Socket);
                break;

            case 'T':
                List_Query_Response(Client_Socket);
                break; 

            case 'F':
                File_Response(Client_Socket);
                break;   

            default:
                std::cout << "[Error de Lectura]: No se ha podido identificar el protocolo a seguir.\n";
                break;

        }

    }

}

/*
=================================
              MAIN
=================================
*/

int main(){

    int Server_Socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in Server_Addr;
    Server_Addr.sin_family = AF_INET;
    Server_Addr.sin_port = htons(45000);
    Server_Addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(Server_Socket, (struct sockaddr*)& Server_Addr, sizeof(Server_Addr)) < 0){

        std::cout << "Error de Bind.\n";
        return -1;

    }
    else{

        listen(Server_Socket, 5);
        std::cout << "==================== Servidor listo para conectarse ====================\n";

    }


    while (true){
        
        sockaddr_in ClientAddr;
        socklen_t ClientLen = sizeof(ClientAddr);

        int Client_Socket = accept(Server_Socket, (struct sockaddr*)& ClientAddr, &ClientLen);

        if(Client_Socket < 0){

            std::cout << "[ERROR]: No se pudo conectar con el cliente, REVISAR.\n";
            break;

        }

        else{

            std::cout << "Conectado con el cliente (" << Client_Socket << ").\n";

        }

        std::thread Client_Thread(Leer_Redirigir, Client_Socket);

        Client_Thread.detach();

    }
    





}