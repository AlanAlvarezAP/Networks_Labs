    /* Server code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>

  #include <iostream>
  #include <string>
  #include <thread>
  #include <unordered_map>

  std::unordered_map<std::string,int> mapita;

  void read_thread(int n,int SocketFD){
    int size_name,size_msg;
    char buffer[256],name[256],msg[256];
    std::string size_save_name,size_save_msg;
    for(;;){
      bzero(buffer,256);
      bzero(name,256);
      bzero(msg,256);
      n = read(SocketFD,buffer,3);
      buffer[n]='\0';
      size_save_name=buffer;
      size_name=std::atoi(buffer);
        
      n = read(SocketFD,buffer,size_name);
      buffer[n]='\0';  
      strcpy(name,buffer);

      n = read(SocketFD,buffer,3);
      buffer[n]='\0';
      size_save_msg=buffer;
      size_msg=std::atoi(buffer);

      n = read(SocketFD,buffer,size_msg);
      buffer[n]='\0';
      strcpy(msg,buffer);

      std::string final_msg=size_save_name+std::string{name}+size_save_msg+std::string{msg};

      int Socket_dest_FD=mapita[name];
      write(Socket_dest_FD,final_msg.data(),final_msg.size());
    }
    

  }

  int main(void)
  {
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
      read(ClientFD,buffer,255);
      std::string nickname=std::string{buffer};
      mapita[nickname]=ClientFD;

      std::thread (read_thread,n,ClientFD).detach();
    }
 
    shutdown(ClientFD, SHUT_RDWR);
 
    close(ClientFD);
 
    close(ServerFD);
    return 0;
  }
