#pragma once

#include <stdint.h>
#include <vector>

class UDPClient
{
public:
  UDPClient(int port);
  ~UDPClient();
  void write(std::vector<uint8_t> data);
  void write(const void *data, int length);

private:
  int sockfd;
};

class UDPServer
{
public:
  UDPServer(int port);
  ~UDPServer();
  std::vector<uint8_t> read();

private:
  int sockfd;
};