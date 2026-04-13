#pragma once
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
#include <unordered_map>
#include <limits>

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

void print_map(std::unordered_map<std::string,int> little_map){
	std::cout << "------------------------------------------MAP STATE -----------------------------" << std::endl;
	for(auto &p:little_map){
		std::cout << " We have person " << p.first << " with socket " << p.second << std::endl;
	}
	std::cout << "---------------------------------------------------------------------------------" << std::endl;

}

class Server_Protocols {
public:
	std::unordered_map<std::string, int> little_map;

	std::string Login(int n, int SocketFD) {
		char buffer[256];

		bzero(buffer, 256);

		n = read(SocketFD, buffer, 4);
		buffer[n] = '\0';

		int size_name = std::atoi(buffer);

		n = read(SocketFD, buffer, size_name);

		return buffer;

	}

	void Cases_Server(char type, int n, int SocketFD) {
		switch (type) {
			case 'L':{
				std::string nickname = Login(n, SocketFD);
				if (little_map.find(nickname) != little_map.end()) {
					std::string error_msg = "ERROR nickname already in server";
					int size_error = error_msg.size();
					std::string final_msg = "E" + number_to_string(size_error, 5) + error_msg;
					write(SocketFD, final_msg.data(), final_msg.size());
				}
				else {
					little_map[nickname] = SocketFD;
					char k = 'K';
					write(SocketFD, &k, 1);
				}

				print_map(little_map);

				break;
			}
			
			case 'O': {
				for (auto it = little_map.begin(); it != little_map.end(); ++it) {
					if (it->second == SocketFD) {
						little_map.erase(it);
						char k = 'K';
						write(SocketFD, &k, 1);
						shutdown(SocketFD, SHUT_RDWR);
						close(SocketFD);
						print_map(little_map);
						return;
					}
				}
				std::string error_msg = "ERROR loginout";
				int size_error = error_msg.size();
				std::string final_msg = "E" + number_to_string(size_error, 5) + error_msg;
				write(SocketFD, final_msg.data(), final_msg.size());
				break;
			}
			default: {
				std::cout << "This protocol is not registered :( " << std::endl;
				break;
			}

		}

	}

};

class Client_Protocols {
public:
	bool logging_status = false,running=false;
	void Error(int n, int SocketFD) {
		char buffer[256];

		n = read(SocketFD, buffer, 5);
		buffer[n] = '\0';

		int size_msg = std::atoi(buffer);
		n = read(SocketFD, buffer, size_msg);
		buffer[n] = '\0';

		std::cout << "ERROR -> " << buffer << std::endl;

	}

	void Login(int n, int SocketFD) {
		std::string name;

		std::cout << "Give me your nickname to send -> ";
		std::getline(std::cin, name);
		int size_msg = name.size();
		std::string final_msg = "L" + number_to_string(size_msg, 4) + name;
		write(SocketFD, final_msg.data(), final_msg.size());

	}

	void Cases_Client(char type, int n, int SocketFD) {
		switch (type) {
			case 'L': {
				Login(n, SocketFD);
				break;
			}
			case 'O': {
				char O = 'O';
				write(SocketFD, &O, 1);
				break;
			}
			case 'K': {
				std::cout << "All good OK " << std::endl;
				logging_status = true;
				break;
			}
			case 'E': {
				Error(n, SocketFD);
				logging_status = false;
				break;
			}

			default: {
				std::cout << "This protocol is not registered :( " << std::endl;
				break;
			}
		}

	}

};
