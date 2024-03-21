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

std::string find_http_path (const std::string& buf) {
  int begin_path = buf.find(' ');

  int end_path = buf.find(' ', begin_path+1);

  std::cout << begin_path << " " << end_path << std::endl;

  std::string path_message = buf.substr(begin_path+1, end_path-4);

  return path_message;

}

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

  std::string buffer;
  buffer.resize(256, '\0');

  ssize_t bytesRead;

  bytesRead = recv(client, (void*) &buffer[0], buffer.max_size(), 0);
  if (bytesRead < 0) {
      std::cerr << "Error reading from socket" << std::endl;
      return -1;
  }


  std::string headers(buffer);

  std::string path_string = find_http_path(headers);

  //std::cout << headers << std::endl;



  //std::cout << path_string << std::endl;

  std::string content_type = "Content-Type: text/plain\r\n";
  std::string content_length = "Content-Length: ";
  
  
  std::string response_message;

  if (path_string == "/") {
    response_message = "HTTP/1.1 200 OK\r\n\r\n";
    //response_message = response_message + content_type + content_length;
  }
  else if (path_string.find("/echo/") != std::string::npos) {
    std::string path_message = path_string.substr(path_string.find('/', 1) + 1);
    response_message = "HTTP/1.1 200 OK\r\n" + content_type + content_length + std::to_string(path_message.size()) + "\r\n" + "\r\n" + path_message;
  }
  else if (path_string.find("/user-agent") != std::string::npos) {
    std::cout << path_string << std::endl;
    ssize_t user_agent_begin = headers.find("User-Agent: ");
    ssize_t user_agent_end = headers.find("\r\n", user_agent_begin);
    std::string user_agent_header = headers.substr(user_agent_begin+strlen("User-Agent: "), user_agent_end-user_agent_begin-strlen("User-Agent: "));
    
    response_message = "HTTP/1.1 200 OK\r\n" + content_type + content_length + std::to_string(user_agent_header.size()) + "\r\n" + "\r\n" + user_agent_header;

  }
  
  else {
    response_message = "HTTP/1.1 404 Not Found\r\n\r\n";
    //response_message = response_message + content_type + content_length;
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
