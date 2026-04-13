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


class Protocols_Receivers {
public:
	
	std::string Login(int n, int SocketFD) {
		int size_name;
		char buffer[256];

		bzero(buffer, 256);
		n = read(SocketFD, buffer, 4);
		buffer[n] = '\0';

		int size_name = std::atoi(buffer);

		n = read(SocketFD, buffer, size_name);

		return buffer;

	}

	void Error(int n, int SocketFD) {
		char buffer[256];

		n = read(SocketFD, buffer, 5);
		buffer[n] = '\0';

		int size_msg = std::atoi(buffer);
		n = read(SocketFD, buffer, size_msg);
		buffer[n] = '\0';

		std::cout << "ERROR -> " << buffer << std::endl;

	}

	void Receive_Protocol(char type, int n, int SocketFD, std::unordered_map<std::string,int>*little_map = nullptr) {
		switch (type) {
			case 'L': {
				std::string nickname=Login(n, SocketFD);
				if (little_map && little_map->find(nickname) != little_map->end()) {
					std::string error_msg = "ERROR nickname already in server";
					int size_error = error_msg.size();
					std::string final_msg = "E" + number_to_string(size_error, 5) + error_msg;
					write(SocketFD, final_msg.data(), final_msg.size());
				}else {
					little_map[nickname] = SocketFD;
					char k = 'K';
					write(SocketFD, &k, 1);
				}

				std::cout << "---------MAPA ACTU-------------" << std::endl;
				for (auto &p : *little_map) {
					std::cout << "Se tiene persona " << p.first << " con socket " << p.second << std::endl;
				}
				std::cout << "-------------------------------" << std::endl;

				break;
			}
			case 'K': {
				std::cout << "All good OK " << std::endl;
				break;
			}
			case 'E': {
				Error(n, SocketFD);
				break;
			}
			case 'O': {
				if (!little_map) {
					std::string error_msg = "ERROR loginout";
					int size_error = error_msg.size();
					std::string final_msg = "E" + number_to_string(size_error, 5) + error_msg;
					write(SocketFD, final_msg.data(), final_msg.size());
				}
				for (auto it = little_map->begin(); it != little_map->end(); ++it) {
					if (it->second == SocketFD) {
						little_map->erase(it);
						char k = 'K';
						write(SocketFD, &k, 1);
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
				std::cout << "This protocolo is not registered :( " << std::endl;
				break;
			}

		}

	}

};


class Protocols_Senders {
public:
	
	void Login(int n, int SocketFD) {
		std::string name;

		std::cout << "Give me your nickname to send -> ";
		std::getline(std::cin, name);
		int size_msg = name.size();
		std::string final_msg = "L" +number_to_string(size_msg,4) + name;
		write(SocketFD, final_msg.data(), final_msg.size());

	}

	void Send_Protocol(char type, int n, int SocketFD) {
		switch(type) {
			case 'L': {
				Login(n, SocketFD);
				break;
			}
			case 'O': {
				char O = 'O';
				write(SocketFD, &O, 1);
				break;
			}
			default: {
				std::cout << "This protocolo is not registered :( " << std::endl;
				break;
			}
		}
	
	}

};