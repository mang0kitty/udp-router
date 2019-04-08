#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include "types.h"
#include "packet.h"
#include "network_table.h"
#include "udp.h"
#include <climits>
#include <unistd.h>
#include <thread>
#include <chrono>

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

void router_run_watchdog(router_id_t id, NetworkTable *networkTable, std::map<router_id_t, std::chrono::system_clock::time_point> *heartbeats)
{
    std::map<router_id_t, NetworkRoute> originalRoutes;

    while (1)
    {
        sleep(1);
        auto now = std::chrono::system_clock::now();
        auto expiresAt = now - std::chrono::seconds(3);

        auto original = networkTable->to_string();
        for (auto &&entry : *heartbeats)
        {
            if (entry.second < expiresAt && originalRoutes.find(entry.first) == originalRoutes.end())
            {
                originalRoutes[entry.first] = networkTable->route(entry.first);

                NetworkRoute route;
                route.source = id;
                route.dest = entry.first;
                route.port = -1;
                route.cost = INT_MAX - 1;

                networkTable->update(&route);
                std::cout << id << ": heartbeat lost for " << entry.first << ": marked route as unavailable" << std::endl;
            }
            else if (entry.second > expiresAt && originalRoutes.find(entry.first) != originalRoutes.end())
            {
                networkTable->update(&originalRoutes[entry.first]);
                originalRoutes.erase(entry.first);
                std::cout << id << ": heartbeat returned for " << entry.first << ": marked route as available again" << std::endl;
            }
        }

        auto updated = networkTable->to_string();
        if (updated.compare(original) != 0)
        {
            auto now = std::chrono::system_clock::now();
            auto now_time = std::chrono::system_clock::to_time_t(now);

            std::cout << "---------------------------------" << std::endl;
            std::cout << std::ctime(&now_time) << std::endl;
            std::cout << "Original Routing Table:" << std::endl;
            std::cout << original << std::endl;
            std::cout << "New Routing Table:" << std::endl;
            std::cout << updated << std::endl;
        }
    }
}

void router_run_propagate(NetworkTable *networkTable)
{
    while (1)
    {
        sleep(1);

        auto neighboursList = networkTable->neighbours();
        auto routingTableList = networkTable->full_table();

        for (auto &&neighbour : neighboursList)
        {
            UDPClient client(neighbour.port);

            DVPacket packet;
            memset(&packet, 0, sizeof(DVPacket));
            packet.header.packetType = DV_PACKET_TYPE;
            packet.header.source = neighbour.source;
            packet.header.dest = neighbour.dest;
            packet.header.ttl = 1;

            int i = 0;
            for (auto &&route : routingTableList)
            {
                packet.sources[i] = route.dest;
                packet.costs[i] = route.cost;
                i++;
                if (i == 50)
                    throw "cannot support more than 50 routes per distance vector packet";
            }

            client.write(&packet, sizeof(packet));
        }
    }
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
    std::map<router_id_t, std::chrono::system_clock::time_point> heartbeats;

    std::thread propagator(router_run_propagate, &networkTable);
    std::thread watchdog(router_run_watchdog, id, &networkTable, &heartbeats);

    while (1)
    {
        auto data = server.read();
        auto header = (const PacketHeader *)(&data[0]);

        if (header->dest != id)
        {
            if (header->ttl == 1)
            {
                std::cout << id << ": failed to forward packet to " << header->dest << ": ttl expired" << std::endl;
                continue;
            }

            // Forward the packet
            auto route = networkTable.route(header->dest);
            if (route.cost == INT_MAX)
            {
                std::cout << id << ": failed to forward packet to " << header->dest << ": no known route" << std::endl;
                continue;
            }

            header->ttl--;

            UDPClient client(route.port);
            client.write(data);
            std::cout << id << ": forwarded packet from " << header->source << " to " << header->dest << " via port " << route.port << " (ttl:" << header->ttl << ")" << std::endl;

            continue;
        }

        switch (header->packetType)
        {
        case CONTROL_PACKET_TYPE:
        {
            auto controlPacket = (const ControlPacket *)(&data[0]);
            std::cout << id << ": received control packet from " << header->source << ": " << controlPacket->update.source << "->" << controlPacket->update.dest << " via " << controlPacket->update.port << " costs " << controlPacket->update.cost << std::endl;

            heartbeats[controlPacket->update.dest] = std::chrono::system_clock::now();
            if (networkTable.update(&controlPacket->update))
            {
                std::cout << id << ": ";
                networkTable.print();
            }
        }
        break;
        case DV_PACKET_TYPE:
        {
            auto dvPacket = (const DVPacket *)(&data[0]);
            heartbeats[dvPacket->header.source] = std::chrono::system_clock::now();

            auto original = networkTable.to_string();

            for (int i = 0; i < sizeof(dvPacket->sources); i++)
            {
                if (!dvPacket->sources[i])
                    break;

                NetworkRoute route;
                route.source = dvPacket->header.source;
                route.dest = dvPacket->sources[i];
                route.cost = dvPacket->costs[i];
                route.port = networkTable.port_to(dvPacket->header.source);

                networkTable.update(&route);
            }

            auto updated = networkTable.to_string();
            if (updated.compare(original) != 0)
            {
                auto now = std::chrono::system_clock::now();
                auto now_time = std::chrono::system_clock::to_time_t(now);

                std::cout << "---------------------------------" << std::endl;
                std::cout << std::ctime(&now_time) << std::endl;
                std::cout << "Original Routing Table:" << std::endl;
                std::cout << original << std::endl;
                std::cout << "New Routing Table:" << std::endl;
                std::cout << updated << std::endl;
            }
        }
        break;
        case DATA_PACKET_TYPE:
        {
            auto dataPacket = (const DataPacket *)(&data[0]);
            std::cout << id << ": received data packet from " << header->source << ": " << dataPacket->payload << std::endl;
        }
        break;
        default:
            std::cout << id << ": unrecognized packet type " << header->packetType << std::endl;
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
    packet.header.ttl = 10;

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
    packet.header.ttl = 10;

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