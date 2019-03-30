#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include "types.h"
#include "packet.h"
#include "network_table.h"
#include "udp.h"
#include <climits>

void print_help()
{
    std::cout << "my-router MODE [ARGS...]" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;
    std::cout << "my-router run ID PORT" << std::endl;
    std::cout << "   e.g. my-router run A 10001" << std::endl;
    std::cout << "my-router send PORT SOURCE DEST PAYLOAD" << std::endl;
    std::cout << "   e.g. my-router send 10001 A D 'Hello world!'" << std::endl;
    std::cout << "my-router configure ID PORT SOURCE DEST DPORT COST" << std::endl;
    std::cout << "   e.g. my-router configure A 10001 A B 10002 7" << std::endl;
}

/**
 * my-router run ID PORT
 * > my-router run A 10000
 */
int router_run(std::vector<std::string> args)
{
    if (args.size() < 2)
    {
        print_help();
        return -1;
    }

    router_id_t id = (router_id_t)*args[0].c_str();
    int port = std::stoi(args[1]);

    // TODO: Add a thread to publish our network table to our neighbours
    // TODO: Move all of this into the router.cpp file
    UDPServer server(port);
    NetworkTable networkTable(id);

    while (1)
    {
        auto data = server.read();
        auto header = (const PacketHeader *)(&data[0]);

        if (header->dest != id)
        {
            // Forward the packet
            auto route = networkTable.route(header->dest);
            if (route.cost == INT_MAX)
            {
                std::cout << "failed to forward packet to " << header->dest << ": no known route" << std::endl;
                continue;
            }

            UDPClient client(route.port);
            client.write(data);

            continue;
        }

        switch (header->packetType)
        {
        case CONTROL_PACKET_TYPE:
        {
            auto controlPacket = (const ControlPacket *)(&data[0]);
            networkTable.update(&controlPacket->update);
            networkTable.print();
        }
        break;
        case DATA_PACKET_TYPE:
        {
            auto dataPacket = (const DataPacket *)(&data[0]);
            std::cout << "received data packet from " << header->source << ": " << dataPacket->payload << std::endl;
        }
        break;
        default:
            std::cout << "unrecognized packet type " << header->packetType << std::endl;
            break;
        }
    }

    // Start a new router (class) which listens on the configured port for control and data packets
    return 0;
}

/**
 * my-router configure ID PORT SOURCE DEST DPORT COST
 * > my-router configure A 10000 A B 10001 4
 */
int router_configure(std::vector<std::string> args)
{
    if (args.size() < 6)
    {
        print_help();
        return -1;
    }

    router_id_t id = (router_id_t)*args[0].c_str();
    int routerPort = std::stoi(args[1]);

    // Network table update
    ControlPacket packet;
    packet.header.packetType = CONTROL_PACKET_TYPE;
    packet.header.source = id;
    packet.header.dest = id;

    packet.update.source = (router_id_t)*args[2].c_str();
    packet.update.dest = (router_id_t)*args[3].c_str();
    packet.update.port = std::stoi(args[4]);
    packet.update.cost = std::stoi(args[5]);

    UDPClient client(routerPort);
    client.write(&packet, sizeof(packet));

    return 0;
}

/**
 * my-router send PORT SOURCE DEST PAYLOAD
 * > my-router send 10000 A D "Hello World"
 */
int router_send(std::vector<std::string> args)
{
    if (args.size() < 2)
    {
        print_help();
        return -1;
    }

    int routerPort = std::stoi(args[0]);

    DataPacket packet;
    packet.header.packetType = DATA_PACKET_TYPE;
    packet.header.source = (router_id_t)*args[1].c_str();
    packet.header.dest = (router_id_t)*args[2].c_str();

    std::string payload = args[3].substr(0, 99); // Limit this to 99 chars (100 including \0)
    memcpy(packet.payload, payload.c_str(), payload.size() + 1);

    UDPClient client(routerPort);
    client.write(&packet, sizeof(packet));

    return 0;
}

int main(int argc, const char **argv)
{
    std::vector<std::string> args;

    for (int i = 0; i < argc; i++)
        args.push_back(std::string(argv[i]));

    if (args.size() < 2)
    {
        print_help();
        return -1;
    }

    std::string mode = args[1];

    if (mode.compare("run") == 0)
        return router_run(std::vector<std::string>(args.begin() + 2, args.end()));

    if (mode.compare("configure") == 0)
        return router_configure(std::vector<std::string>(args.begin() + 2, args.end()));

    if (mode.compare("send") == 0)
        return router_send(std::vector<std::string>(args.begin() + 2, args.end()));

    print_help();
    return -1;
}