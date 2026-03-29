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

  void  number_to_char(int number,char result[]){
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


  }
 
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    char buffer[256];
    char buffer_2[256];
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
 
    char test[3];
    number_to_char(999,test);
    printf("%s\n",test);

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
    for(;;){
      bzero(buffer_2,256);
      bzero(buffer,256);
      printf("Message to the server -> ");
      fgets(buffer_2,sizeof(buffer_2),stdin);
      buffer_2[strcspn(buffer_2,"\n")]='\0';
      n = write(SocketFD,(void*)buffer_2,strlen(buffer_2));
      /* perform read write operations ... */
      n = read(SocketFD,buffer,255);
      buffer[n]='\0';
      printf("Server: [%s]\n",buffer);
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
  }
