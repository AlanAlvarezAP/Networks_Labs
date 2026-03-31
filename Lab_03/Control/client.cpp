  /* Client code in C */
 
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
 
  void write_thread(int n,int SocketFD,std::string msg_2){
    std::string size_name,name,size_msg,msg;
    name=msg_2;
    std::cout << "enter msg: ";
    std::getline(std::cin,msg);

    size_name=number_to_string((int)name.size());
    size_msg=number_to_string((int)msg.size());
    std::string final_msg=size_name+name+size_msg+msg;

    n = write(SocketFD,final_msg.data(),final_msg.size());
     
  }

  void read_thread(int n,int SocketFD){
    int size_name,size_msg;
    char buffer[256],name[256],msg[256];
    for(;;){
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

      std::cout << "msg with destination to: " << name  << " -> " << msg << std::endl;
    }
     
  }

  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in));

    std::string own_nickname;
    std::cout << "Give me your nickname :D -> ";
    std::getline(std::cin,own_nickname);

    n = write(SocketFD,own_nickname.data(),own_nickname.size());

    std::thread(read_thread,n,SocketFD).detach();
    for(;;){
      std::string msg;
      std::getline(std::cin,msg);
      if(msg == "exit"){
        break;
      }
      write_thread(n,SocketFD,msg);
    }
   
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
  }
