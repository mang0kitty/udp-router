#include "router.h"

Router::Router(router_id_t id, int port, std::ostream *log)
{
    this->id = id;
    this->port = port;
    this->log = log;
}

router_id_t Router::getId()
{
    return this->id;
}

std::ostream &Router::log()
{
    return *this->logStream;
}

void Router::sendPacket(const PacketHeader *packet, size_t packetSize)
{
    // TODO: Send the packet to the required router/on the required path based on the
    //       destination and the network table (where it will find the traverse port and shortest path).
}

void Router::processPacket(const PacketHeader *packet)
{
    switch (packet->packetType)
    {
    case CONTROL_PACKET_TYPE:
        /* code */
        this->processControlPacket((ControlPacket *)packet);
        break;

    case DATA_PACKET_TYPE:
        this->processDataPacket((DataPacket *)packet);
        break;

    default:
        break;
    }
}
void Router::processControlPacket(const ControlPacket *packet)
{
    log() << "Received control packet: " << packet->update.source << "->" << packet->update.dest << "@" << packet->update.port << " (costs " << packet->update.cost << ")" << std::endl;
    // TODO: Handle the control packet by updating the network table
}
void Router::processDataPacket(const DataPacket *packet)
{
    log() << "Received data packet: " << packet->header.source << "->" << packet->header.dest << ": " << packet->payload << std::endl;

    // TODO: Handle the incoming data packet by either forwarding it (if the destination is not this router)
    //       or by printing it out to our log file if the destination is this router.
}