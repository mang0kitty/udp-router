#pragma once
#include <vector>
#include <map>
#include <tuple>
#include "types.h"

typedef struct network_table_update
{
    router_id_t source;
    router_id_t dest;
    int port;
    int cost;
} NetworkTableUpdate;

class NetworkTable
{
  public:
    int cost(router_id_t source, router_id_t dest);
    std::vector<network_table_update> update(network_table_update *info);

  private:
    std::map<std::tuple<router_id_t, router_id_t>, int> traversePort;

    // Lowest cost between two nodes on the network
    std::map<std::tuple<router_id_t, router_id_t>, int> pathCost;
};