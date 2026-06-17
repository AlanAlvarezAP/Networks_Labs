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
#include <chrono>
#include <sys/stat.h>

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
	    std::string content, file_name, dest, orig;
	    long size_content, size_file_name, size_dest, size_orig;
	 
	    const long MAX_SIZE = 999999999;
	    char size_buf[11];
	 
	    if(read(SocketFD, size_buf, 10) <= 0){
	        return;
	    }
	    size_buf[10] = '\0';
	 
	    size_content = std::atol(size_buf);
	    if(size_content > MAX_SIZE){
	         return;
	    }
	    char buffer[500];
	    long total = 0;
	    int chunk_num = 0;
	    content.clear();
	    content.reserve(size_content);
	 
	    while(total < size_content){
	        long to_read = std::min(500L, size_content - total);
	        int n = read(SocketFD, buffer, to_read);
	 
	        if(n <= 0){
	            return;
	        } 
	 
	        content.append(buffer, n);
	        total += n;
	        chunk_num++;
	 
	        if(chunk_num == 1){
	            std::cout << "First block sent with " << readed << " bytes with content " << buffer << std::endl;
	        }
	    }
	 
	    if(read(SocketFD, size_buf, 5) <= 0){
	        return;
	    } 
	    size_buf[5] = '\0';
	 
	    size_file_name = std::atoi(size_buf);
	 
	    total = 0;
	    file_name.clear();
	 
	    while(total < size_file_name){
	        long to_read = std::min(10L, size_file_name - total);
	        int n = read(SocketFD, buffer, to_read);
	 
	        if(n <= 0) return;
	 
	        file_name.append(buffer, n);
	        total += n;
	    }
	 
	    if(read(SocketFD, size_buf, 5) <= 0) return;
	    size_buf[5] = '\0';
	 
	    size_dest = std::atoi(size_buf);
	 
	    total = 0;
	    dest.clear();
	 
	    while(total < size_dest){
	        long to_read = std::min(10L, size_dest - total);
	        int n = read(SocketFD, buffer, to_read);
	 
	        if(n <= 0) return;
	 
	        dest.append(buffer, n);
	        total += n;
	    }
	 
	    if(little_map.find(dest) == little_map.end()){
	        std::string error_msg = "ERROR destination for file not found :( ";
	        std::string final_msg = "E" + number_to_string((long)error_msg.size(), 5) + error_msg;
	        write(SocketFD, final_msg.data(), final_msg.size());
	        return;
	    }
	 
	    for(auto it = little_map.begin(); it != little_map.end(); ++it){
	        if(it->second == SocketFD){
	            orig = it->first;
	            break;
	        }
	    }
	 
	    std::string final_msg =
	        "f"
	        + number_to_string(size_content, 10)
	        + content
	        + number_to_string((long)size_file_name, 5)
	        + file_name
	        + number_to_string((long)orig.size(), 5)
	        + orig;
	 
	    
	    if(final_msg.size() > 500) {
	        std::string last_500 = final_msg.substr(final_msg.size() - 500);
			std::cout << "First block sent with " << readed << " bytes with content " << last_500 << std::endl;
	    }
	 
	    write(little_map[dest], final_msg.data(), final_msg.size());
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

	void Send_File(int n, int SocketFD, std::string file_name, std::string destination){
	    const long MAX_SIZE = 999999999;
	 
	    std::ifstream file(file_name, std::ios::binary);
	    if(!file.is_open()) {
	        std::cerr << "Error: no se puede abrir archivo" << std::endl;
	        return;
	    }
	 
	    std::string msg;
	    msg.reserve(MAX_SIZE);
	 
	    char buffer[500];
	    int chunk_num = 0;
	    long total_leido = 0;
	    
	    while(file && msg.size() < MAX_SIZE){
	        file.read(buffer, std::min(500L, MAX_SIZE - (long)msg.size()));
	        int readed = file.gcount();
	        chunk_num++;
	 
	        if(chunk_num == 1){
	            std::cout << "First block sent with " << readed << " bytes with content " << buffer << std::endl;
	        }
	        
	        if(readed <= 0){
	            break;
	        } 
	 
	        msg.append(buffer, readed);
	        total_leido += readed;
	    }
	 
	    file.close();
	 
	    std::cout << "Total de bytes leidos del archivo: " << total_leido << std::endl << std::endl;
	 
	    if(file_name.size() > MAX_SIZE)
	        file_name.resize(MAX_SIZE);
	 
	    if(destination.size() > MAX_SIZE)
	        destination.resize(MAX_SIZE);
	 
	    std::string final_msg =
	        "F"
	        + number_to_string((long)msg.size(), 10)
	        + msg
	        + number_to_string((long)file_name.size(), 5)
	        + file_name
	        + number_to_string((long)destination.size(), 5)
	        + destination;
	 
	    if(final_msg.size() > 500) {
	        std::string last_500 = final_msg.substr(final_msg.size() - 500);
			std::cout << "First block sent with " << readed << " bytes with content " << last_500 << std::endl;
	    }
	 
	    write(SocketFD, final_msg.data(), final_msg.size());
	}
	void File_read(int n,int SocketFD){
	    std::string file_name, origin;
	    long size_file, size_file_name, size_orig;
	 
	    const long MAX_SIZE = 999999999;
	    char size_buf[11];
	 
	    if(read(SocketFD, size_buf, 10) <= 0){
			return;
		}
	    size_buf[10] = '\0';
	 
	    size_file = std::atol(size_buf);
	    if(size_file > MAX_SIZE){
	        return;
	    } 
	 
	    std::string file;
	    file.reserve(size_file);
	 
	    char buffer[500];
	    int chunk_num = 0;
	    long total = 0;
	    
	    while(total < size_file){
	        long to_read = std::min(500L, size_file - total);
	        int n = read(SocketFD, buffer, to_read);
	 
	        if(n <= 0){
	            return;
	        } 
	 
	        file.append(buffer, n);
	        total += n;
	        chunk_num++;
	 
	        if(chunk_num == 1){
	            std::cout << "Primer bloque recibido en File_read: " << n << " bytes" << std::endl;
	        }
	    }
	 
	    if(read(SocketFD, size_buf, 5) <= 0){
	        return;
	    } 
	    size_buf[5] = '\0';
	 
	    size_file_name = std::atoi(size_buf);
	 
	    total = 0;
	    file_name.clear();
	 
	    while(total < size_file_name){
	        long to_read = std::min(10L, size_file_name - total);
	        int n = read(SocketFD, buffer, to_read);
	 
	        if(n <= 0) {
	            return;
	        }
	 
	        file_name.append(buffer, n);
	        total += n;
	    }
	 
	    if(read(SocketFD, size_buf, 5) <= 0){
	        return;
	    } 
	    size_buf[5] = '\0';
	 
	    size_orig = std::atoi(size_buf);
	 
	    total = 0;
	    origin.clear();
	 
	    while(total < size_orig){
	        long to_read = std::min(10L, size_orig - total);
	        int n = read(SocketFD, buffer, to_read);
	 
	        if(n <= 0) {
	            return;
	        }
	        origin.append(buffer, n);
	        total += n;
	    }
	 
	    // Construir el mensaje recibido para mostrar los primeros y últimos 500
	    std::string protocolo_recibido = 
	        "f"
	        + number_to_string(size_file, 10)
	        + file
	        + number_to_string(size_file_name, 5)
	        + file_name
	        + number_to_string(size_orig, 5)
	        + origin;
	 
	    if(protocolo_recibido.size() > 500) {
	        std::string last_500 = final_msg.substr(final_msg.size() - 500);
			std::cout << "First block sent with " << readed << " bytes with content " << last_500 << std::endl;
	    }
	 
	    std::cout << "FILE: " << file_name << std::endl << "FROM: " << origin << std::endl;
	 
	    std::ofstream ofs("2" + file_name, std::ios::binary);
	    ofs.write(file.data(), file.size());
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
