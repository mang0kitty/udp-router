#pragma once
#include <iostream>
#include "types.h"
#include "packet.h"
#include "network_table.h"

class Router
{
  public:
    Router(router_id_t id, int port, std::ostream *log);
    router_id_t getId();

  protected:
    void processPacket(const PacketHeader *packet);
    void processControlPacket(const ControlPacket *packet);
    void processDataPacket(const DataPacket *packet);

    void sendPacket(const PacketHeader *packet, size_t packetSize);
    std::ostream &log();

  private:
    router_id_t id;
    int port;
    NetworkTable networkTable;
    std::ostream *logStream;
};