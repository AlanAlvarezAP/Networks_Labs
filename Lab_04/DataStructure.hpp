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
		buffer[n]='\0';
		return buffer;

	}
	
	void Broadcast(int n, int SocketFD) {
		char buffer[256];
		bzero(buffer, 256);

		n = read(SocketFD, buffer, 7);
		buffer[n] = '\0';

		int size_msg = std::atoi(buffer);
		n = read(SocketFD, buffer, size_msg);
		buffer[n] = '\0';


		std::string author;
		int size_author;
		for (auto it = little_map.begin(); it != little_map.end(); ++it) {
			if (it->second == SocketFD) {
				author = it->first;
				size_author = author.size();
				break;
			}
		}

		std::string broadcast_msg = "b" + number_to_string(size_author, 3) + author + number_to_string(size_msg, 7) + std::string{ buffer };
		
		for (auto it = little_map.begin(); it != little_map.end(); ++it) {
			write(it->second, broadcast_msg.data(), broadcast_msg.size());
		}

	}

	void Unicast(int n, int SocketFD) {
		char buffer[256];
		bzero(buffer, 256);

		n = read(SocketFD, buffer, 5);
		buffer[n] = '\0';

		int size_msg = std::atoi(buffer);

		n = read(SocketFD, buffer, size_msg);
		buffer[n] = '\0';

		std::string msg=buffer;

		n = read(SocketFD, buffer, 7);
		buffer[n] = '\0';

		int size_dst = std::atoi(buffer);

		n = read(SocketFD, buffer, size_dst);
		buffer[n] = '\0';

		std::string destination=buffer;

		if (little_map.find(destination) == little_map.end()) {
			std::string error_msg = "ERROR destination not in the server";
			int size_error = error_msg.size();
			std::string final_msg = "E" + number_to_string(size_error, 5) + error_msg;
			write(SocketFD, final_msg.data(), final_msg.size());
			return;
		}

		std::string author;
		int size_auth;
		for (auto it = little_map.begin(); it != little_map.end(); ++it) {
			if (it->second == SocketFD) {
				author = it->first;
				size_auth = author.size();
				break;
			}
		}

		int SocketDST;
		for (auto it = little_map.begin(); it != little_map.end(); ++it) {
			if (it->first == destination) {
				SocketDST = it->second;
				break;
			}
		}

		std::string final_msg = "u" + number_to_string(size_auth, 7) + author + number_to_string(size_msg, 5) + msg;
		write(SocketDST, final_msg.data(), final_msg.size());
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
			case 'B': {
				Broadcast(n, SocketFD);
				break;
			}
			case 'U': {
				Unicast(n, SocketFD);
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

	void Broadcast(int n, int SocketFD) {
		std::string msg;

		std::cout << "Give me the message to everyone -> ";
		std::getline(std::cin, msg);
		int size_msg = msg.size();
		std::string final_msg = "B" + number_to_string(size_msg, 7) + msg;
		write(SocketFD, final_msg.data(), final_msg.size());
	}

	void Broadcast_react(int n, int SocketFD) {
		char buffer[256];
		bzero(buffer, 256);

		n = read(SocketFD, buffer, 3);
		buffer[n] = '\0';

		int size_author = std::atoi(buffer);

		n = read(SocketFD, buffer, size_author);
		buffer[n] = '\0';

		std::string author=buffer;

		n = read(SocketFD, buffer, 7);
		buffer[n] = '\0';

		int size_msg = std::atoi(buffer);

		n = read(SocketFD, buffer, size_msg);
		buffer[n] = '\0';

		std::string msg= buffer;

		std::cout << "Message from: " << author << " with a message of " << msg << std::endl;

	}

	void Unicast(int n, int SocketFD) {
		std::string msg,nickname_dest;
		std::cout << "Give me the msg to be sent ";
		std::getline(std::cin, msg);

		int size_msg = msg.size();

		std::cout << "Give me the destination " << std::endl;
		std::getline(std::cin, nickname_dest);

		int size_dst = nickname_dest.size();

		std::string final_msg = "U" + number_to_string(size_msg, 5) + msg + number_to_string(size_dst, 7) + nickname_dest;
		write(SocketFD, final_msg.data(), final_msg.size());

	}

	void Unicast_react(int n, int SocketFD) {
		char buffer[256];
		bzero(buffer, 256);

		n = read(SocketFD, buffer, 7);
		buffer[n] = '\0';

		int size_origin = std::atoi(buffer);
		n = read(SocketFD, buffer, size_origin);
		buffer[n] = '\0';

		std::string origin=buffer;

		n = read(SocketFD, buffer, 5);
		buffer[n] = '\0';

		int size_msg = std::atoi(buffer);
		n = read(SocketFD, buffer, size_msg);
		buffer[n] = '\0';

		std::string msg=buffer;

		std::cout << "Message from: " << origin << " with a message of -> " << msg << std::endl;

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
				if (logging_status == true) {
					std::cout << " I entered case logging out " << logging_status << " and running " << running << std::endl;
					logging_status=false;
					running = false;
				}
				else {
					std::cout << " I entered the other case where i am logging in" << std::endl;
					logging_status = true;
				}
				break;
			}
			case 'E': {
				Error(n, SocketFD);
				logging_status = false;
				break;
			}
			case 'B': {
				Broadcast(n, SocketFD);
				break;
			}
			case 'b': {
				Broadcast_react(n, SocketFD);
				break;
			}
			case 'U': {
				Unicast(n, SocketFD);
				break;
			}
			case 'u': {
				Unicast_react(n, SocketFD);
				break;
			}
			default: {
				std::cout << "This protocol is not registered :( " << std::endl;
				break;
			}
		}

	}

};
