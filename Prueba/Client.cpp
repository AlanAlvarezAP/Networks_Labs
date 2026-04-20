#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <mutex>

/*
=================================
  Variables Globales de Control
=================================
*/

bool Running = true;
std::string NickName_Local;
int Local_Socket;

std::mutex Mutex_Consola;

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

void Print_Mutex(const std::string& Msg){

    std::lock_guard<std::mutex> lock(Mutex_Consola);
    std::cout << Msg;

}

void Print_Mensaje(const std::string& Usuario, const std::string& Msg){

    std::lock_guard<std::mutex> lock(Mutex_Consola);

    std::cout << "\n[" << Usuario << "]: " << Msg << "\n";
    std::cout << "[" << NickName_Local << "]: ";
    std::cout.flush();

}



/*
=================================
Funciones de Lectura del Servidor
=================================
*/

void Okey_Msg(){

    std::lock_guard<std::mutex> lock(Mutex_Consola);
    std::cout << "La accion realizada se completo satisfactoriamente.\n";

}

void Error_Msg(){

    char Buffer_Size[6];
    
    read(Local_Socket, Buffer_Size, 5);

    Buffer_Size[5] = '\0';
    int Error_Size = atoi(Buffer_Size);

    std::string Msg;
    Msg.resize(Error_Size);

    read(Local_Socket, &Msg[0], Error_Size);

    Print_Mutex("[Servidor]:" + Msg + "\n");

}

void Unicast_Read(){

    char Buffer_Size[8];
    
    read(Local_Socket, Buffer_Size, 7);

    Buffer_Size[7] = '\0';
    int NickName_Origen_Size = atoi(Buffer_Size);

    std::string NickName_Origen;
    NickName_Origen.resize(NickName_Origen_Size);

    read(Local_Socket, &NickName_Origen[0], NickName_Origen_Size);


    read(Local_Socket, Buffer_Size, 5);
    
    Buffer_Size[5] = '\0';
    int Msg_Size = atoi(Buffer_Size);

    std::string Msg;
    Msg.resize(Msg_Size);

    read(Local_Socket, &Msg[0], Msg_Size);

    Print_Mensaje(NickName_Origen, Msg);

}

void Broadcast_Read(){

    char Buffer_Size[8];
    
    read(Local_Socket, Buffer_Size, 3);

    Buffer_Size[3] = '\0';
    int NickName_Origen_Size = atoi(Buffer_Size);

    std::string NickName_Origen;
    NickName_Origen.resize(NickName_Origen_Size);

    read(Local_Socket, &NickName_Origen[0], NickName_Origen_Size);


    read(Local_Socket, Buffer_Size, 7);
    
    Buffer_Size[7] = '\0';
    int Msg_Size = atoi(Buffer_Size);

    std::string Msg;
    Msg.resize(Msg_Size);

    read(Local_Socket, &Msg[0], Msg_Size);

    Print_Mensaje(NickName_Origen, Msg);

}

void List_Read(){};
void File_Read(){};

/*
=================================
Funciones de Opciones del Cliente
=================================
*/

void Login_Protocol(){

    std::cout << "Ingrese su Nickname para ser registrado: ";
    std::getline(std::cin, NickName_Local);

    if(NickName_Local.length() > 9999){

    std::cout << "[Error] Nickname demasiado largo, truncado al maximo de tamano.\n";
    NickName_Local.resize(9999);

    }
    
    std::string Packet = "L" + Num_to_Str(NickName_Local.length(), 4) + NickName_Local;

    write(Local_Socket, Packet.c_str(), Packet.length());

}

void Logout_Protocol(){

    write(Local_Socket, "O", 1);

}

void Unicast_Send(){

    std::string NickName_Destino;
    std::string Msg;

    std::cout << "Ingrese el Nickname del destinatario:";
    std::getline(std::cin, NickName_Destino);
    
    if(NickName_Destino.length() > 9999999){

    std::cout << "[Error] Nickname demasiado largo, truncado al maximo de tamano.\n";
    NickName_Destino.resize(9999999);
    
    }
    
    std::cout << "[Mensaje a Enviar]: ";
    std::getline(std::cin, Msg);

    if(Msg.empty()){
        
    std::cout << "Mensaje vacío, no enviado.\n";
    return;

    }
    
    if(Msg.length() > 99999){

    std::cout << "[Error] Mensaje demasiado largo, truncado al maximo de tamano.\n";
    Msg.resize(99999);
    
    }

    std::string Packet = "U" + Num_to_Str(Msg.length(), 5) + Msg
                       + Num_to_Str(NickName_Destino.length(), 7) + NickName_Destino;
    
    write(Local_Socket, Packet.c_str(), Packet.length());          
    
}

void Broadcast_Send(){

    std::string Msg;

    std::cout << "[Mensaje Global]:";
    std::getline(std::cin, Msg);

    if(Msg.empty()){

    std::cout << "Mensaje vacío, no enviado.\n";
    return;

    }

    if(Msg.length() > 9999999){

    std::cout << "[Error] Mensaje demasiado largo, truncado al maximo de tamano.\n";
    Msg.resize(9999999);
    
    }

    std::string Packet = "B" + Num_to_Str(Msg.length(), 7) + Msg;

    write(Local_Socket, Packet.c_str(), Packet.length());

}

void List_Query(){}
void File_Send(){}

void Lectura(int Client_Socket){

    while(Running){

        char Protocolo;
        int n = read(Client_Socket, &Protocolo, 1);

        if(n <= 0){

            Print_Mutex("Servidor desconectado.\n");

            Running = false;
            break;

        }

        switch (Protocolo){

        case 'K':
            Okey_Msg();
            break;

        case 'E':
            Error_Msg();
            break; 

        case 'u':
            Unicast_Read();
            break; 

        case 'b':
            Broadcast_Read();
            break;

        case 'l':
            List_Read();
            break; 

        case 'f':
            File_Read();
            break;   

        default:
            std::cout << "[Error de Lectura]: No se ha podido identificar el protocolo a seguir.\n";
            break;

        }

    }

}

void Mostrar_Menu(){

    std::lock_guard<std::mutex> lock(Mutex_Consola);

    std::cout << "============ REDES - CHAT-O ============\n";
    std::cout << "[L] - Login\n";
    std::cout << "[O] - Logout\n";
    std::cout << "[U] - Unicast\n";
    std::cout << "[B] - Broadcast\n";
    std::cout << "[T] - Solicitar Lista de Clients\n";
    std::cout << "[F] - Enviar Archivo\n";
    std::cout << "[E] - Exit\n";
    std::cout << "========================================\n";
}

void Construir_Interfaz(){

    char Opcion;

    {
        std::lock_guard<std::mutex> lock(Mutex_Consola);
        std::cout << "[" << NickName_Local << "]: ";
    }

    std::cin >> Opcion;
    std::cin.ignore();

    switch (Opcion) {
        case 'L': case 'l':
            Login_Protocol();
            break;

        case 'O': case 'o':
            Logout_Protocol();
            break;

        case 'U': case 'u':
            Unicast_Send();
            break;

        case 'B': case 'b':
            Broadcast_Send();
            break;

        case 'T': case 't':
            List_Query();
            break;

        case 'F': case 'f':
            File_Send();
            break;

        case 'E': case 'e':
            Logout_Protocol();
            Running = false;
            break;

        default:
            Print_Mutex("Opcion invalida\n");
            break;
    }
}


/*
=================================
              MAIN
=================================
*/

int main(){

    int Client_Socket = socket(AF_INET, SOCK_STREAM, 0);
    Local_Socket = Client_Socket;
    
    sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(45000);

    inet_pton(AF_INET, "127.0.0.1", &SockAddr.sin_addr);

    if(connect(Client_Socket, (struct sockaddr*)& SockAddr, sizeof(SockAddr)) < 0){

        std::cout << "Error de conexion con el servidor.\n";
        return -1;

    }
    else{

        std::cout << "Conectado con el servidor. Disfrute.\n";

    }

    std::thread Lectura_Cliente(Lectura, Client_Socket);

    Mostrar_Menu();

    while(Running){

        Construir_Interfaz();

    }

    Lectura_Cliente.join();
    close(Client_Socket);

}