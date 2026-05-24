#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <cstring>

#define PORT 45000



std::string number_to_string(int number,int size) {
	std::string result(size, ' ');
	int count = size-1;

	if (number < 0) {
		number = -number;
	}

	while (number > 0) {
		int division = number % 10;
		result[count--] = division + '0';
		number /= 10;
	}

	while (count >= 0) {
		result[count--] = '0';
	}

	return result;

}



void send_fragmented(int sock, const std::string& msg) {
    for (size_t i = 0; i < msg.size(); i += 3) {
        std::string chunk = msg.substr(i, 3);

        send(sock, chunk.c_str(), chunk.size(), 0);

        std::cout << "[Fragment sent]: " << chunk << std::endl;

        usleep(100000); // 100 ms (Artificial latency)
    }
}

void tests(int SocketFD){
    // Send fragmented texts
    std::cout << "----------- TEST 1 ---------------------";
    std::cout << "----------- Normal/Correct JSON---------";
    std::string msg="{\"Name\":\"Paquito\",\"Age\":20}";
    std::string msg_to_test=number_to_string(msg.size(),5)+"J"+msg;
    send_fragmented(SocketFD,msg_to_test);

    std::cout << "----------- TEST 2 ---------------------";
    std::cout << "----------- Parser wrong Identifier---------";
    msg="{\"Product\":\"Laptop\",\"Price\":3500,\"Stock Size\":15}";
    msg_to_test=number_to_string(msg.size(),5)+"K"+msg;
    send_fragmented(SocketFD,msg_to_test);


    std::cout << "----------- TEST 3 ---------------------";
    std::cout << "----------- Parser wrong Json start '{'---------";
    msg="\"Product\":\"Laptop\",\"Price\":3500,\"Stock Size\":15}";
    msg_to_test=number_to_string(msg.size(),5)+"J"+msg;
    send_fragmented(SocketFD,msg_to_test);

    std::cout << "----------- TEST 4 ---------------------";
    std::cout << "----------- Parser wrong Json start '}'---------";
    msg="{\"Product\":\"Laptop\",\"Price\":3500,\"Stock Size\":15";
    msg_to_test=number_to_string(msg.size(),5)+"J"+msg;
    send_fragmented(SocketFD,msg_to_test);

    std::cout << "----------- TEST 5 ---------------------" << std::endl;
    std::cout << "----------- Desconection kill -9 ---------" << std::endl;
    kill(getpid(),SIGKILL);

}

int main() {
    int sock = 0;
    sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

    std::cout << "Connected with the server succesfully :D" << std::endl;

    tests(sock);

    sleep(1);


    close(sock);

    return 0;
}