#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <map>
#include <mutex>

#define DEFAULT_PORT 1601
#define BUFFER_SIZE 1024
#define CLIENT_CLOSE_CONNECTION_SYMBOL '#'

std::map<int, std::string> clients;
std::mutex clients_mtx;

void Broadcast(const std::string& message, int sender_fd){
    clients_mtx.lock();
    for(auto it = clients.begin(); it != clients.end(); ++it){
        int sock = it->first;
        if(sock != sender_fd){
            send(sock, message.c_str(), message.length(), 0);
        }
    }

    clients_mtx.unlock();
}




void server_connection(int client_fd){
    char buffer[BUFFER_SIZE];
    std::string name;

    const char* msg_ask = "Enter your name: \n";
    send(client_fd, msg_ask, strlen(msg_ask), 0);
    
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(client_fd, buffer, BUFFER_SIZE,0);
    if(bytes <= 0){
        return;
    }
    name = std::string(buffer);

    clients_mtx.lock();
    clients[client_fd] = name;
    clients_mtx.unlock();

    std::cout << "User " << name << " connected." << std::endl;
    Broadcast("Server: " + name + " has joined the chat.\n", client_fd);
    
    std::string welcome = "Welcome to the chat, " + name + "!\n";
    send(client_fd, welcome.c_str(), welcome.length(), 0);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            std::cout << "Connection lost.\n";
            break;
        }

        std::cout << buffer << std::endl;

        if (buffer[0] == CLIENT_CLOSE_CONNECTION_SYMBOL) {
            break;
        }
        
        std::string raw_msg = std::string(buffer);
        clients_mtx.lock();
        if(raw_msg[0] == '@'){
            int ind = raw_msg.find(' ');
            if(ind != std::string::npos){
                std::string target_name = raw_msg.substr(1,ind-1);
                std::string text = raw_msg.substr(ind+1);

                std::string f_msg = "from [" + name + "]: " + text + "\n";
                
                bool found = false;
                for(auto it = clients.begin(); it != clients.end(); ++it){
                    if(it->second == target_name){
                        send(it->first, f_msg.c_str(), f_msg.length(), 0);
                        found = true;
                        break;
                    }
                }
                
                if(!found){
                    std::string err = "Server: User not found\n";
                
                }
                
            }
            else{
                std::string err = "Server: Empty message\n";
                send(client_fd, err.c_str(), err.length(), 0);
            }

        }
        else{
            std::string msg = name + ": " + std::string(buffer) + "\n";
            for(auto it = clients.begin(); it != clients.end(); ++it){
                int sock = it->first;
                if(sock != client_fd){
                    send(sock, msg.c_str(), msg.length(), 0);
                }
            }
        }
        clients_mtx.unlock();
    }
    clients_mtx.lock();
    clients.erase(client_fd);
    clients_mtx.unlock();

    std::cout << "\nGoodBye..." << std::endl;

}

void server_thread(int client_fd){
    auto thread_id = std::this_thread::get_id();
    std::cout << "Thread" << thread_id << "socket: " << client_fd << std::endl;

    server_connection(client_fd);

    std::cout << "Thread done" << std::endl;
    close(client_fd);
}

int main(int argc, char* const argv[]) {
    int server_fd;

    struct sockaddr_in server_address;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cout << "ERROR: establishing socket error\n";
        return -1;
    }

    std::cout << "SERVER: Socket created\n";

    server_address.sin_port = htons(DEFAULT_PORT);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);

    int ret = bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address));

    if (ret < 0) {
        std::cout << "ERROR: binding connection\n";
        return -1;
    }

    std::cout << "SERVER: Listening...\n";
    listen(server_fd, 5);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int newsockfd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        
        if(newsockfd < 0){
            std::cout << "Error on accpet" << std::endl;
            continue;
        }

        std::thread client_thr(server_thread, newsockfd);

        client_thr.detach();

    }
}
