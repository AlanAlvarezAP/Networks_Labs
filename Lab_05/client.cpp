#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


struct TickTackToe {
    char cell_moves[9];
    char curr_move;
    char winner_draw;
};

void check_win(struct TickTackToe &dat){
    for(int i=0;i<=6;i+=3){
        if((dat.cell_moves[i] == dat.cell_moves[i+1]) && (dat.cell_moves[i] == dat.cell_moves[i+2])){
            dat.winner_draw='w';
            return;
        }    
    }

    for(int i=0;i<=2;i++){
        if((dat.cell_moves[i] == dat.cell_moves[i+3]) && (dat.cell_moves[i] == dat.cell_moves[i+6])){
            dat.winner_draw='w';
            return;
        }    
    }
    if((dat.cell_moves[0] == dat.cell_moves[4]) && (dat.cell_moves[0] == dat.cell_moves[8])){
        dat.winner_draw='w';
        return;
    }
    if((dat.cell_moves[2] == dat.cell_moves[4]) && (dat.cell_moves[2] == dat.cell_moves[6])){
        dat.winner_draw='w';
        return;
    }            
}

void check_draw(struct TickTackToe &dat){
    for(int i=0;i<9;i++){
        if(dat.cell_moves[i] != '-'){
            dat.winner_draw='d';
            return;
        }
    }
}

int main() {

    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(45000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    struct TickTackToe data = {{'-','-','-','-','-','-','-','-','-'},'X',false};

    while(true){
        write(sock, (char*)&data, sizeof(data));
        printf("Struct sent\n");


    }


    close(sock);

    return 0;
}
