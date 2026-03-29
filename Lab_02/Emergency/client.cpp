  /* Client code in C */
 
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
 
  void write_thread(int n,int SocketFD){
	 std::string size_name,name,size_msg,msg;
	 std::cout << "nickname of the destination: ";
	 std::getline(std::cin,name);
	 std::cout << "enter msg: ";
	 std::getline(std::cin,msg);

	 size_name=number_to_string((int)name.size());
	 size_msg=number_to_string((int)msg.size());

	 std::string final_msg=size_name+name+size_msg+msg;
	 
	 n = write(SocketFD,final_msg.data(),final_msg.size());
     
  }

  void read_thread(char buffer[],int n,int SocketFD){
     int size_name,size_msg;
     char name[256],msg[256];

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
		
	     std::cout << "msg from: " << name << std::endl;
	     std::cout << "msg: " << msg << std::endl;
	 }
     
  }

  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    char buffer[256];
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;

    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

	std::thread(read_thread,buffer,n,SocketFD).detach();
	for(;;){
		write_thread(n,SocketFD);
	}
   
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
  }
