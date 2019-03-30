#pragma once
#include "types.h"
#include "network_table.h"

typedef struct packet_header
{
    packet_t packetType;
    router_id_t source;
    router_id_t dest;
} PacketHeader;

const packet_t CONTROL_PACKET_TYPE = 1;

/**
 * Send a routing table update from Router A to Router D
 * indicating that the cost to traverse A->B is 4 over port 10001.
 * 
 * ------------
 * 01 41 44 41 
 * 42 00 00 27
 * 11 00 00 00
 * 04
 * ------------
 * 
 * 01 = Control Packet
 * 41 = Router A
 * 44 = Router D
 * -------------
 * 41 = Router A
 * 42 = Router B
 * 00002711 = Port 10001
 * 00000004 = Cost of 4
 */
typedef struct control_packet
{
    PacketHeader header;
    NetworkRoute update;
} ControlPacket;

const packet_t DATA_PACKET_TYPE = 2;

/**
 * Send a payload from Router A to Router D
 * --------
 * 02 41 44
 * ........
 * 
 * --------
 * 02 = Data Packet
 * 41 = Router A
 * 44 = Router D
 */
typedef struct data_packet
{
    PacketHeader header;
    char payload[100];
} DataPacket;
