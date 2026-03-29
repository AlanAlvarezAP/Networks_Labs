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

  std::string number_to_string(int number){
     std::string result(3);
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

  void write_thread(int &n,int SocketFD){
     std::string size_name,name,size_msg,msg;

     std::cout << "nickname of destination: ";
     std::getline(std::cin,name);
     std::cout << std::endl;
     std::cout << "enter msg: ";
     std::getline(std::cin,msg);
     std::cout << std::endl;

     size_name=number_to_string((int)name.size());
     size_msg=number_to_string((int)msg.size());

     std::string final_msg=size_name+name+size_msg+msg;
     
     n = write(SocketFD,final_msg.data(),final_msg.size());

  }

  void read_thread(char buffer[],int &n,int SocketFD){
	   for(;;){
		   int size_name,size_msg;
	     std::string name,msg;
	
	     bzero(buffer,256);
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
	
	     std::cout << "msg from: " << name << std::endl;
	     std::cout << "msg: " << msg << std::endl;
	   }
  }

  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int ServerFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    int n;
 
    if(-1 == ServerFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(ServerFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(ServerFD);
      exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(ServerFD, 10))
    {
      perror("error listen failed");
      close(ServerFD);
      exit(EXIT_FAILURE);
    }
    
    int ClientFD=accept(ServerFD,NULL,NULL);
    std::thread listener(read_thread,buffer,n,SocketFD).detach();

    for(;;){
      std::thread writer(write_thread,n,SocketFD);

      writer.join();
    }
 
   shutdown(ClientFD, SHUT_RDWR);
 
   close(ClientFD);
 
   close(ServerFD);
   return 0;
  }
