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
#include <sstream>
#include "json.hpp"

typedef nlohmann::json json;

std::string number_to_string_2(int number, int size) {
    std::string result(size, ' ');
    int count = size - 1;
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

void print(const std::unordered_map<std::string,sockaddr_in>& clientes){
	for(const auto& cliente : clientes){
	    std::cout << "ID: " << cliente.first << std::endl;
	}
}

class Server_Protocols_UDP {
public:
    std::unordered_map<std::string, sockaddr_in> client_map;

    std::string Login(const std::string& buffer, int server_socket, sockaddr_in& client_addr) {
        std::string size_str = buffer.substr(1, 4);
        int size_name = std::atoi(size_str.c_str());
        
        std::string nickname = buffer.substr(5, size_name);
        if (client_map.find(nickname) != client_map.end()) {
            std::string error_msg = "ERROR nickname already in server";
            int size_error = error_msg.size();
            std::string final_msg = "E" + number_to_string_2(size_error, 5) + error_msg;
            sendto(server_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
        } else {
            std::cout << "Hola papu" << std::endl;
            client_map[nickname] = client_addr;
            char k = 'K';
            sendto(server_socket, &k, 1, 0, (sockaddr*)&client_addr, sizeof(client_addr));
	        print(client_map);
        }
        
        return nickname;
    }

    void Broadcast(const std::string& buffer, int server_socket, sockaddr_in& client_addr) {
        std::string size_str = buffer.substr(1, 7);
        int size_msg = std::atoi(size_str.c_str());
        
        std::string msg = buffer.substr(8, size_msg);
        
        std::string author;
        for (auto& pair : client_map) {
            if (pair.second.sin_addr.s_addr == client_addr.sin_addr.s_addr && pair.second.sin_port == client_addr.sin_port) {
                author = pair.first;
                break;
            }
        }
        
        int size_author = author.size();
        std::string broadcast_msg = "b" + number_to_string_2(size_author, 3) + author + number_to_string_2(size_msg, 7) + msg;
        
        for (const auto& pair : client_map) {
            sendto(server_socket, broadcast_msg.data(), broadcast_msg.size(), 0, (sockaddr*)&(pair.second), sizeof(pair.second));
        }
    }

    void Unicast(const std::string& buffer, int server_socket, sockaddr_in& client_addr) {
        std::string size_str = buffer.substr(1, 5);
        int size_msg = std::atoi(size_str.c_str());
        
        std::string msg = buffer.substr(6, size_msg);
        
        size_str = buffer.substr(6 + size_msg, 7);
        int size_dst = std::atoi(size_str.c_str());
        
        std::string destination = buffer.substr(6 + size_msg + 7, size_dst);

        if (client_map.find(destination) == client_map.end()) {
            std::string error_msg = "ERROR destination not in the server";
            int size_error = error_msg.size();
            std::string final_msg = "E" + number_to_string_2(size_error, 5) + error_msg;
            sendto(server_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
            return;
        }
        
        std::string author;
        for (const auto& pair : client_map) {
            if (pair.second.sin_addr.s_addr == client_addr.sin_addr.s_addr && pair.second.sin_port == client_addr.sin_port) {
                author = pair.first;
                break;
            }
        }
        
        int size_auth = author.size();
        sockaddr_in dst_addr = client_map.find(destination)->second;
        
        std::string final_msg = "u" + number_to_string_2(size_auth, 7) + author + number_to_string_2(size_msg, 5) + msg;
        sendto(server_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&dst_addr, sizeof(dst_addr));
    }

    void Send_List(int server_socket, sockaddr_in& client_addr) {
        json js;
        js["clients"] = json::array();
        for (const auto& pair : client_map) {
            js["clients"].push_back(pair.first);
        }
        std::string to_send = js.dump();
        std::string final_msg = "t" + number_to_string_2((int)to_send.size(), 5) + to_send;
        sendto(server_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
    }

    void File_redirect(const std::string& buffer, int server_socket, sockaddr_in& client_addr) {
        const int MAX_SIZE = 99999;
        
        std::string size_str = buffer.substr(1, 5);
        int size_content = std::atoi(size_str.c_str());
        
        if (size_content > MAX_SIZE || buffer.length() < 1 + 5 + size_content + 5){
			return;
		}
        
        std::string content = buffer.substr(6, size_content);
        
        size_str = buffer.substr(6 + size_content, 5);
        int size_file_name = std::atoi(size_str.c_str());
        
        std::string file_name = buffer.substr(6 + size_content + 5, size_file_name);
        
        size_str = buffer.substr(6 + size_content + 5 + size_file_name, 5);
        int size_dest = std::atoi(size_str.c_str());
        std::string dest = buffer.substr(6 + size_content + 5 + size_file_name + 5, size_dest);
		
        if (client_map.find(dest) == client_map.end()) {
            std::string error_msg = "ERROR destination for file not found :( ";
            std::string final_msg = "E" + number_to_string_2(error_msg.size(), 5) + error_msg;
            sendto(server_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
            return;
        }
        
        std::string orig;
        for (auto& pair : client_map) {
            if (pair.second.sin_addr.s_addr == client_addr.sin_addr.s_addr && pair.second.sin_port == client_addr.sin_port) {
                orig = pair.first;
                break;
            }
        }
        std::string final_msg = "f" + number_to_string_2(size_content, 5) + content + number_to_string_2(size_file_name, 5) + file_name + number_to_string_2(orig.size(), 5) + orig;
        
        sockaddr_in dst_addr = client_map.find(dest)->second;
        sendto(server_socket, final_msg.data(), final_msg.size(), 0,(sockaddr*)&dst_addr, sizeof(dst_addr));
    }

    void Logout(int server_socket, sockaddr_in& client_addr) {
        std::string author;
        for (auto it = client_map.begin(); it != client_map.end(); ++it) {
            if (it->second.sin_addr.s_addr == client_addr.sin_addr.s_addr && it->second.sin_port == client_addr.sin_port) {
                author = it->first;
                client_map.erase(it);
                char k = 'K';
                sendto(server_socket, &k, 1, 0, (sockaddr*)&client_addr, sizeof(client_addr));
		        print(client_map);
                return;
            }
        }
        
        std::string error_msg = "ERROR logout";
        int size_error = error_msg.size();
        std::string final_msg = "E" + number_to_string_2(size_error, 5) + error_msg;
        sendto(server_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
    }

    void Cases_Server(const std::string& buffer, int server_socket, sockaddr_in& client_addr) {
        char type = buffer[0];
        switch (type) {
            case 'L': {
                Login(buffer, server_socket, client_addr);
                break;
            }
            case 'O': {
                Logout(server_socket, client_addr);
                break;
            }
            case 'B': {
                Broadcast(buffer, server_socket, client_addr);
                break;
            }
            case 'U': {
                Unicast(buffer, server_socket, client_addr);
                break;
            }
            case 'T': {
                Send_List(server_socket, client_addr);
                break;
            }
            case 'F': {
                File_redirect(buffer, server_socket, client_addr);
                break;
            }
            default: {
                std::cout << "This protocol is not registered in Server :( " << std::endl;
                break;
            }
        }
    }
};

class Client_Protocols_UDP {
public:
    bool logging_status = false, running = false;

    void Error(const std::string& buffer) {
        std::string size_str = buffer.substr(1, 5);
        int size_msg = std::atoi(size_str.c_str());
        
        std::string error_msg = buffer.substr(6, size_msg);
        std::cout << "ERROR -> " << error_msg << std::endl;
    }

    void Login(int client_socket, sockaddr_in& server_addr) {
        std::string name;
        std::cout << "Give me your nickname to send -> ";
        std::getline(std::cin, name);
        int size_msg = name.size();
        std::string final_msg = "L" + number_to_string_2(size_msg, 4) + name;
        
        sendto(client_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
    }

    void Broadcast(int client_socket, sockaddr_in& server_addr) {
        std::string msg;
        std::cout << "Give me the message to everyone -> ";
        std::getline(std::cin, msg);
        int size_msg = msg.size();
        std::string final_msg = "B" + number_to_string_2(size_msg, 7) + msg;
        
        sendto(client_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
    }

    void Broadcast_react(const std::string& buffer) {
        std::string size_str = buffer.substr(1, 3);
        int size_author = std::atoi(size_str.c_str());
        
        std::string author = buffer.substr(4, size_author);
        
        size_str = buffer.substr(4 + size_author, 7);
        int size_msg = std::atoi(size_str.c_str());
        
        std::string msg = buffer.substr(4 + size_author + 7, size_msg);
        
        std::cout << "Message from: " << author << " with a message of " << msg << std::endl;
    }

    void Unicast(int client_socket, sockaddr_in& server_addr) {
        std::string msg, nickname_dest;
        std::cout << "Give me the msg to be sent ";
        std::getline(std::cin, msg);
        int size_msg = msg.size();
        std::cout << "Give me the destination " << std::endl;
        std::getline(std::cin, nickname_dest);
        int size_dst = nickname_dest.size();
        std::string final_msg = "U" + number_to_string_2(size_msg, 5) + msg + number_to_string_2(size_dst, 7) + nickname_dest;
        
        sendto(client_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
    }

    void Unicast_react(const std::string& buffer) {
        std::string size_str = buffer.substr(1, 7);
        int size_origin = std::atoi(size_str.c_str());
        
        std::string origin = buffer.substr(8, size_origin);
        
        size_str = buffer.substr(8 + size_origin, 5);
        int size_msg = std::atoi(size_str.c_str());
        
        std::string msg = buffer.substr(8 + size_origin + 5, size_msg);
        
        std::cout << "Message from: " << origin << " with a message of -> " << msg << std::endl;
    }

    void JSON_react(const std::string& buffer) {
        std::string size_str = buffer.substr(1, 5);
        int size_json = std::atoi(size_str.c_str());
        
        std::string json_str = buffer.substr(6, size_json);
        json js = json::parse(json_str);
        std::cout << js.dump(4) << std::endl;
    }

    void Send_File(int client_socket, sockaddr_in& server_addr, std::string file_name, std::string destination) {
        const int MAX_SIZE = 99999;
        std::ifstream file(file_name, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Error: Could not open file " << file_name << std::endl;
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string msg = buffer.str();
        
        if (msg.size() > MAX_SIZE) {
            msg.resize(MAX_SIZE);
        }
        if (file_name.size() > MAX_SIZE){
			file_name.resize(MAX_SIZE);
		}
        if (destination.size() > MAX_SIZE){
			destination.resize(MAX_SIZE);
		}
        
        std::string final_msg = "F" + number_to_string_2(msg.size(), 5) + msg + number_to_string_2(file_name.size(), 5) + file_name + number_to_string_2(destination.size(), 5) + destination;
        
        sendto(client_socket, final_msg.data(), final_msg.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));
    }

    void File_read(const std::string& buffer) {
        const int MAX_SIZE = 99999;
        
        std::string size_str = buffer.substr(1, 5);
        int size_file = std::atoi(size_str.c_str());
        
        std::string file = buffer.substr(6, size_file);
        
        size_str = buffer.substr(6 + size_file, 5);
        int size_file_name = std::atoi(size_str.c_str());
        
        std::string file_name = buffer.substr(6 + size_file + 5, size_file_name);
        
        size_str = buffer.substr(6 + size_file + 5 + size_file_name, 5);
        int size_orig = std::atoi(size_str.c_str());
        
        std::string origin = buffer.substr(6 + size_file + 5 + size_file_name + 5, size_orig);
        
        std::cout << "FILE: " << file_name << std::endl << "FROM: " << origin << std::endl;
        std::ofstream ofs("received_" + file_name, std::ios::binary);
        ofs.write(file.data(), file.size());
    }

    void Cases_Client_UDP(const std::string& buffer, int client_socket, sockaddr_in& server_addr) {
		char type=buffer[0];
        switch (type) {
            case 'L': {
                Login(client_socket, server_addr);
                break;
            }
            case 'O': {
                char O = 'O';
                logging_status = false;
                running = false;
                sendto(client_socket, &O, 1, 0, (sockaddr*)&server_addr, sizeof(server_addr));
                break;
            }
            case 'B': {
                Broadcast(client_socket, server_addr);
                break;
            }
            case 'U': {
                Unicast(client_socket, server_addr);
                break;
            }
            case 'T': {
                char T = 'T';
                sendto(client_socket, &T, 1, 0, (sockaddr*)&server_addr, sizeof(server_addr));
                break;
            }
            case 'F': {
                std::string dest, file_nam;
                std::cout << "Give me the file name ";
                std::getline(std::cin, file_nam);
                std::cout << " Give me the destination ";
                std::getline(std::cin, dest);
                Send_File(client_socket, server_addr, file_nam, dest);
                break;
            }
			case 'K': {
                std::cout << "All good OK " << std::endl;
                if (logging_status == true) {
                    logging_status = false;
                    running = false;
                } else {
                    logging_status = true;
                }
                break;
            }
            case 'E': {
                Error(buffer);
                break;
            }
            case 'b': {
                Broadcast_react(buffer);
                break;
            }
            case 'u': {
                Unicast_react(buffer);
                break;
            }
            case 't': {
                JSON_react(buffer);
                break;
            }
            case 'f': {
                File_read(buffer);
                break;
            }
            default: {
                std::cout << "This protocol is not registered in Client :( " << std::endl;
                break;
            }
        }
    }
};
