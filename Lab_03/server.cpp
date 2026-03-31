    /* Server code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>

  #include <iostream>
  #include <string>
  #include <thread>
  #include <unordered_map>

  std::unordered_map<std::string,int> mapita;

  std::string number_to_string(int number){
    std::string result(3,' ');
    int count=2;

    if (number < 0){
      number=-number;
    }
    
    while(number > 0){
      int division=number%10;
      result[count--]=division+'0';
      number/=10;
    }

    while(count >= 0){
      result[count--]='0';
    }
    return result;

  }

  void read_thread(char buffer[],int n,int SocketFD){
    int size_name,size_msg;
    char name[256],msg[256];


    bzero(buffer,256);
    bzero(name,256);
    bzero(msg,256);
    n = read(SocketFD,buffer,3);
    buffer[n]='\0';
    size_name=std::atoi(buffer);
      
    n = read(SocketFD,buffer,size_name);
    buffer[n]='\0';  
    strcpy(name,buffer);

    n = read(SocketFD,buffer,3);
    buffer[n]='\0';
    size_msg=std::atoi(buffer);

    n = read(SocketFD,buffer,size_msg);
    buffer[n]='\0';
    strcpy(msg,buffer);

    std::string final_msg=std::to_string(size_name)+std::string{name}+std::to_string(size_msg)+std::string{msg};
    
    for(auto p:mapita){
      write(p.second,final_msg.data(),final_msg.size());
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
      std::thread (read_thread,buffer,n,ClientFD).detach();
    }
 
    shutdown(ClientFD, SHUT_RDWR);
 
    close(ClientFD);
 
    close(ServerFD);
    return 0;
  }
