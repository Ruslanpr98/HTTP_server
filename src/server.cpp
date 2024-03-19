#include <iostream>
#include <cstdlib>
#include <string>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 256

int main(int argc, char **argv) {
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  int server_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - Communication between processes by IPV4, SOCK_STREAM - TCP connection, 0 - for interner protocol
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  //
  // // Since the tester restarts your program quite often, setting REUSE_PORT
  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  int client = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  if (client < 0) {
    std::cerr << "Error client acception\n";
    return -1;
  }
  std::cout << "Client connected\n";

  char buffer[BUFFER_SIZE];
  ssize_t bytesRead;

  bytesRead = read(client, buffer, BUFFER_SIZE - 1);
  if (bytesRead < 0) {
      std::cerr << "Error reading from socket" << std::endl;
      return -1;
  }

  std::string headers(buffer);

  //std::cout << headers << std::endl;

  int begin_path = headers.find("echo") + 5;

  int end_path = headers.find(' ', begin_path);

  std::string path_string = headers.substr(begin_path, end_path - begin_path);


  std::cout << path_string << std::endl;

  std::string content_type = "Content-Type: text/plain\r\n\r\n";
  std::string content_length = "Content-Length: 3\r\n\r\n";

  
  
  std::string response_message;
  if (headers.find("/echo") != std::string::npos) {
    response_message = "HTTP/1.1 200 OK\r\n\r\n";
    response_message = response_message + content_type + content_length + path_string;
    
  }
  else if (path_string == "/") {
    response_message = "HTTP/1.1 200 OK\r\n\r\n";
    response_message = response_message + content_type + content_length;
  }
  else {
    response_message = "HTTP/1.1 404 Not Found\r\n\r\n";
    response_message = response_message + content_type + content_length;
  }

  
  
  std::cout << response_message << std::endl;

  

  int response = send(client, response_message.c_str(), response_message.length(), 0);
    if (response < 0) {
        std::cerr << "Error responding to client\n";
        return -1;
    }
  std::cout << "Response send\n";
  
  close(server_fd);

  return 0;
}
