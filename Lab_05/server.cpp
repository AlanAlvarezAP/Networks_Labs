#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct TickTackToe {
    char cell_moves[9];
    char curr_move;
    char winner_draw;
};

void check_win(struct TickTackToe &dat){
    for(int i=0;i<=6;i+=3){
        if(dat.curr_move != dat.cell_moves[i]){
            continue;
        }
        if((dat.cell_moves[i] == dat.cell_moves[i+1]) && (dat.cell_moves[i] == dat.cell_moves[i+2])){
            dat.winner_draw='w';
            return;
        }    
    }

    for(int i=0;i<=2;i++){
        if(dat.curr_move != dat.cell_moves[i]){
            continue;
        }
        if((dat.cell_moves[i] == dat.cell_moves[i+3]) && (dat.cell_moves[i] == dat.cell_moves[i+6])){
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
        if(dat.cell_moves[i] == '-'){
            return;
        }
    }
    dat.winner_draw='d';
}

void print_table(struct TickTackToe &dat){
    for(int i=0;i<9;i+=3){
        printf("%c ",dat.cell_moves[i]);
        printf("%c ",dat.cell_moves[i+1]);
        printf("%c\n",dat.cell_moves[i+2]);
    }
}

void print_info(struct TickTackToe &dat){
    printf("Received struct:\n");
    printf("The table status is:\n");
    print_table(dat);
    printf("\n");
    printf("the turn is = %c\n", dat.curr_move);
    printf("the status of the match is = %c\n", dat.winner_draw);
}

int main() {

    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(45000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));

    listen(server_fd, 5);

    printf("Server waiting...\n");

    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);

    struct TickTackToe data;

    do{
        ssize_t total = 0;
        ssize_t n;
        while (total < sizeof(data)) {

            n = read(client_fd,((char*)&data) + total,sizeof(data) - total);

            if (n <= 0) {
                printf("Connection closed or error\n");
                break;
            }

            total += n;
        }
        print_info(data);

        if(data.winner_draw == 'w'){
            printf("---------------------------- RESULT-----------------------\n");
            printf("The winner is the client :D\n");
            printf("---------------------------- CLIENT WINS -----------------------\n");
            print_table(data);
            break;
        }else if(data.winner_draw == 'd'){
            printf("---------------------------- RESULT-----------------------\n");
            printf("DRAW -_-\n");
            printf("---------------------------- DRAW -----------------------\n");
            print_table(data);
            break;
        }

        int cell;
        do{
            printf("Give me the cell to play (1-9) ");
            scanf("%d",&cell);
        }while((cell < 1 || cell > 9) || data.cell_moves[cell-1] != '-');

        data.cell_moves[cell-1]=data.curr_move;
        check_win(data);
        check_draw(data);
        data.curr_move='X';

        if(data.winner_draw == 'w'){
            printf("---------------------------- RESULT-----------------------\n");
            printf("The winner is the server :D\n");
            printf("---------------------------- SERVER WINS -----------------------\n");
            print_table(data);
            write(client_fd, (char*)&data, sizeof(data));
        }else if(data.winner_draw == 'd'){
            printf("---------------------------- RESULT-----------------------\n");
            printf("DRAW -_-\n");
            printf("---------------------------- DRAW -----------------------\n");
            print_table(data);
            write(client_fd, (char*)&data, sizeof(data));
        }else{
            write(client_fd, (char*)&data, sizeof(data));
            printf("Struct sent\n");
        }

    }while(data.winner_draw != 'w' && data.winner_draw != 'd');


    close(client_fd);
    close(server_fd);

    return 0;
}
