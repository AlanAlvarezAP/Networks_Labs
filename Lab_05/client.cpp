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
        if((dat.curr_move == dat.cell_moves[i])&&(dat.cell_moves[i] == dat.cell_moves[i+1]) && (dat.cell_moves[i] == dat.cell_moves[i+2])){
            dat.winner_draw='w';
            return;
        }    
    }

    for(int i=0;i<=2;i++){
        if((dat.curr_move == dat.cell_moves[i])&&(dat.cell_moves[i] == dat.cell_moves[i+3]) && (dat.cell_moves[i] == dat.cell_moves[i+6])){
            dat.winner_draw='w';
            return;
        }    
    }
    if((dat.curr_move == dat.cell_moves[0])&&(dat.cell_moves[0] == dat.cell_moves[4]) && (dat.cell_moves[0] == dat.cell_moves[8])){
        dat.winner_draw='w';
        return;
    }
    if((dat.curr_move == dat.cell_moves[2])&&(dat.cell_moves[2] == dat.cell_moves[4]) && (dat.cell_moves[2] == dat.cell_moves[6])){
        dat.winner_draw='w';
        return;
    }            
}

void check_draw(struct TickTackToe &dat){
    for(int i=0;i<9;i++){
        if(dat.cell_moves[i] != '-'){
            return;
        }
    }
    dat.winner_draw='d';
}

int main() {

    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(45000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    struct TickTackToe data = {{'-','-','-','-','-','-','-','-','-'},'X','-'};

    while(data.winner_draw != 'w' && data.winner_draw != 'd'){
        int cell;
        do{
            printf("Give me the cell to play (1-9) ");
            scanf("%d",&cell);
        }while((cell <= 0 && cell >= 10) || data.cell_moves[cell-1] != '-');

        data.cell_moves[cell-1]=data.curr_move;
        check_win(data);
        check_draw(data);

        data.curr_move='O';

        write(sock, (char*)&data, sizeof(data));
        printf("Struct sent\n");


        ssize_t total = 0;
        ssize_t n;
        while (total < sizeof(data)) {

            n = read(sock,((char*)&data) + total,sizeof(data) - total);

            if (n <= 0) {
                printf("Connection closed or error\n");
                break;
            }

            total += n;
        }
        printf("Received struct:\n");
        printf("The table status is:\n");
        for(int i=0;i<9;i+=3){
            printf("%c ",data.cell_moves[i]);
            printf("%c ",data.cell_moves[i+1]);
            printf("%c\n",data.cell_moves[i+2]);
        }
        printf("\n");
        printf("the turn is = %c\n", data.curr_move);
        printf("the status of the match is = %c\n", data.winner_draw);


    }

    close(sock);

    return 0;
}
