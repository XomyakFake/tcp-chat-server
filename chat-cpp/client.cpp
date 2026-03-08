#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>    
#include <arpa/inet.h>  
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <cstring>     
#include <thread>

#define ERROR_S "SERVER ERROR: "
#define SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 1601
#define SERVER_CLOSE_CONNECTION_SYMBOL '#'
#define BUFFER_SIZE 1024


bool is_client_connection_close(const char* msg);

void Reciever(int client){
    char buffer[BUFFER_SIZE];
    while(true){
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytes = recv(client, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            std::cout << "Connection closed by server.\n";
            exit(0);
        }

        std::cout << "\r" << buffer << std::endl;
        std::cout << "You:" << std::flush;
    }
}



int main(int argc, const char* argv[]) {
    int client;
    struct sockaddr_in server_address;

   
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        std::cout << ERROR_S << "establishing socket error." << std::endl;
        return -1;
    }

   
    server_address.sin_port = htons(DEFAULT_PORT);
    server_address.sin_family = AF_INET;
    
  
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

    std::cout << "\nClient socket created.\n";

    int ret = connect(client, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));

    if (ret == 0) {
        std::cout << "Connection to server "
                  << inet_ntoa(server_address.sin_addr)
                  << " with port number: " << DEFAULT_PORT << "\n";
    } else {
        std::cout << "Could not connect to server.\n";
        return -1;
    }

    char buffer[BUFFER_SIZE];

    std::cout << "Waiting for server confirmation...\n";
    
    std::cout << "Connection established.\n"
              << "Enter " << SERVER_CLOSE_CONNECTION_SYMBOL
              << " in the first place to close the connection\n";


    memset(buffer,0,BUFFER_SIZE);
    recv(client, buffer, BUFFER_SIZE, 0);
    std::cout << buffer;
    std::cin.getline(buffer, BUFFER_SIZE);
    send(client, buffer, BUFFER_SIZE, 0);

    memset(buffer,0,BUFFER_SIZE);
    recv(client, buffer, BUFFER_SIZE, 0);
    std::cout << buffer;

    std::thread t(Reciever, client);
    t.detach();

    while (true) {
        std::cout << "You: ";
        std::cout.flush();

        std::cin.getline(buffer, BUFFER_SIZE);
        
        send(client, buffer, BUFFER_SIZE, 0);

        if (is_client_connection_close(buffer)) {
            break;
        }
    }


    close(client);
    std::cout << "\nGoodBye..." << std::endl;

    return 0;
}

bool is_client_connection_close(const char* msg) {
    for (int i = 0; i < strlen(msg); ++i) {
        if (msg[i] == SERVER_CLOSE_CONNECTION_SYMBOL) {
            return true;
        }
    }
    return false;
}

