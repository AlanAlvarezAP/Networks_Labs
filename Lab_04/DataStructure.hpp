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
#include <fstream>

#include "json.hpp"

typedef nlohmann::json json;

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

	void Send_List(int n,int SocketFD){
		json js;
		js["clients"]=json::array();
		
		for(auto p:little_map){
			js["clients"].push_back(p.first);
		}

		std::string to_send=js.dump();
		std::string final_msg="t"+number_to_string((int)to_send.size(),5)+to_send;
		write(SocketFD,final_msg.data(),final_msg.size());

	}

	void File_redirect(int n,int SocketFD){
		std::string content,file_name,dest,orig;
		int size_content,size_file_name,size_dest,size_orig;

		char buffer[99999];
		bzero(buffer,99999);
		
		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_content=std::atoi(buffer);

		n=read(SocketFD,buffer,size_content);
		buffer[n]='\0';

		content=buffer;

		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_file_name=std::atoi(buffer);

		n=read(SocketFD,buffer,size_file_name);
		buffer[n]='\0';

		file_name=buffer;

		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_dest=std::atoi(buffer);

		n=read(SocketFD,buffer,size_dest);
		buffer[n]='\0';

		dest=buffer;

		if(little_map.find(dest) == little_map.end()){
			std::string error_msg="ERROR destination for file not found :( ";
			int size_error= error_msg.size();
			std::string final_msg="E"+number_to_string(size_error,5)+error_msg;
			write(SocketFD,final_msg.data(),final_msg.size());
			return;
		}

		for(auto it=little_map.begin();it != little_map.end();it++){
			if(it->second == SocketFD){
				orig=it->first;
				size_orig=orig.size();
				break;
			}
		}

		std::string final_msg="f"+number_to_string(size_content,5)+content+number_to_string(size_file_name,5)+file_name+number_to_string(size_orig,5)+orig;
		write(little_map[dest],final_msg.data(),final_msg.size());
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
			case 'T':{
				Send_List(n,SocketFD);
				break;
			}
			case 'F':{
				File_redirect(n,SocketFD);
				break;
			}
			default: {
				std::cout << "This protocol is not registered in Server :( " << std::endl;
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

	void JSON_react(int n,int SocketFD){
		char buffer[99999];
		int size_json;
		bzero(buffer,99999);
		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_json=std::atoi(buffer);

		n=read(SocketFD,buffer,size_json);
		buffer[n]='\0';

		json js=json::parse(buffer);

		std::cout << js.dump(4) << std::endl;
	}

	void Send_File(int n,int SocketFD,std::string file_name,std::string destination){

		std::ifstream file(file_name+".txt",std::ios::binary);

		char buffer[11];
		bzero(buffer,11);

		std::string msg;


		while(file){
			file.read(buffer,10);

			int readed=file.gcount();

			if(readed == 0){
				break;
			}
			msg+=std::string(buffer,readed);

		}
		std::string final_msg="F"+number_to_string((int)msg.size(),5)+msg+number_to_string((int)file_name.size(),5)+file_name+number_to_string((int)destination.size(),5)+destination;
		write(SocketFD,final_msg.data(),final_msg.size());
	}
	void File_read(int n,int SocketFD){
		std::string file,file_name,origin;
		int size_file,size_file_name,size_orig;

		char buffer[99999];
		bzero(buffer,99999);

		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_file=std::atoi(buffer);

		n=read(SocketFD,buffer,size_file);
		buffer[n]='\0';

		file=buffer;

		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_file_name=std::atoi(buffer);

		n=read(SocketFD,buffer,size_file_name);
		buffer[n]='\0';

		file_name=buffer;

		n=read(SocketFD,buffer,5);
		buffer[n]='\0';

		size_orig=std::atoi(buffer);

		n=read(SocketFD,buffer,size_orig);
		buffer[n]='\0';

		origin=buffer;

		std::cout << " --------------THE FILE SENDED IS CALLED -> " << file_name << "-----------------" << std::endl;
		std::cout << " --------------SENDED  BY: " << origin << "-------------------------------------" << std::endl;
		std::cout << " --------------THE CONTENT IS: -----------------------------------------------" << std::endl;
		std::cout << file << std::endl;

		std::ofstream ofs("bible2.txt", std::ios::binary);

		size_t total = file.size();
		size_t pos = 0;

		while (pos < total) {
			size_t chunk_size = std::min((size_t)10, total - pos);

			ofs.write(file.data() + pos, chunk_size);

			pos += chunk_size;
		}

	}

	void Cases_Client(char type, int n, int SocketFD) {
		switch (type) {
			case 'L': {
				Login(n, SocketFD);
				break;
			}
			case 'O': {
				char O = 'O';
				logging_status=false;
				running=false;
				write(SocketFD, &O, 1);
				close(SocketFD);
				break;
			}
			case 'K': {
				std::cout << "All good OK " << std::endl;
				if (logging_status == true) {
					logging_status=false;
					running = false;
				}
				else {
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
			case 'T':{
				char T='T';
				write(SocketFD,&T,1);
				break;
			}
			case 't':{
				JSON_react(n,SocketFD);
				break;
			}
			case 'F':{
				std::string dest,file_nam;
				std::cout << "Give me the file name ";
				std::getline(std::cin,file_nam);
				std::cout << " Give me the destination ";
				std::getline(std::cin,dest);
				Send_File(n,SocketFD,file_nam,dest);
				break;
			}
			case 'f':{
				File_read(n,SocketFD);
				break;
			}
			default: {
				std::cout << "This protocol is not registered in Client :( " << std::endl;
				break;
			}
		}

	}

};
