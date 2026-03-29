  /* Server code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
 
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
    for(;;)
    {
 
      bzero(buffer,256);
      n = read(ClientFD,buffer,255);
      if (n < 0) perror("ERROR reading from socket");
      buffer[n]='\0';
      printf("Client: [%s]\n",buffer);

      char buffer_2[256];
      bzero(buffer_2,256);
      printf("Message to the client -> ");
      fgets(buffer_2,sizeof(buffer_2),stdin);
      buffer_2[strcspn(buffer_2,"\n")]='\0';
      n = write(ClientFD,(void*)buffer_2,strlen(buffer_2));
      if (n < 0) perror("ERROR writing to socket");
 
   }
 
   shutdown(ClientFD, SHUT_RDWR);
 
   close(ClientFD);
 
   close(ServerFD);
   return 0;
  }
