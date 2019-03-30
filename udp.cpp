#include "udp.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

UDPServer::UDPServer(int port)
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(this->sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        throw "unable to bind to new socket";
    }
}

UDPServer::~UDPServer()
{
    close(this->sockfd);
}

std::vector<uint8_t> UDPServer::read()
{
    struct sockaddr *recv_addr;
    size_t recv_addr_size;

    uint8_t buffer[8196];

    auto received = recv(this->sockfd, buffer, sizeof(buffer), 0);
    if (received <= 0)
    {
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> result(buffer, buffer + received);
    return result;
}

UDPClient::UDPClient(int port)
{
    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (connect(this->sockfd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    {
        throw "unable to bind to new socket";
    }
}

UDPClient::~UDPClient()
{
    close(this->sockfd);
}

void UDPClient::write(std::vector<uint8_t> data)
{
    send(this->sockfd, &data[0], data.size(), 0);
}

void UDPClient::write(const void *data, int length)
{
    send(this->sockfd, data, length, 0);
}