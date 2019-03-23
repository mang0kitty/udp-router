#include <string>
#include <vector>
#include "types.h"
#include "packet.h"
#include "network_table.h"

void print_help()
{
    // TODO: Print out how to use our app
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

    // Start a new router (class) which listens on the configured port for control and data packets
    return 0;
}

/**
 * my-router configure ROUTER_PORT SOURCE DEST PORT COST
 * > my-router configure 10000 A B 10001 4
 */
int router_configure(std::vector<std::string> args)
{
    if (args.size() < 5)
    {
        print_help();
        return -1;
    }

    int routerPort = std::stoi(args[0]);

    // Network table update
    ControlPacket packet;
    packet.header.packetType = CONTROL_PACKET_TYPE;
    packet.header.source = '\0';
    packet.header.dest = '\0';

    packet.update.source = (router_id_t)*args[1].c_str();
    packet.update.dest = (router_id_t)*args[2].c_str();
    packet.update.port = std::stoi(args[3]);
    packet.update.cost = std::stoi(args[4]);

    // TODO: Connect to the router over routerPort and send the control packet to it.

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

    // TODO: Connect to the router over routerPort and send the data packet to it
    //       for forwarding to the destination router.

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