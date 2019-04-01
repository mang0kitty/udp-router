#pragma once
#include <vector>
#include <map>
#include <set>
#include "types.h"

typedef struct network_table_update
{
  router_id_t source;
  router_id_t dest;
  int port;
  int cost;
} NetworkRoute;

class NetworkTable
{
public:
  NetworkTable(router_id_t root);
  bool update(const NetworkRoute *info);
  NetworkRoute route(router_id_t dest);
  void print();
  std::vector<NetworkRoute> neighbours();
  std::vector<NetworkRoute> full_table();

private:
  NetworkRoute backtrack(router_id_t dest);
  int cost(router_id_t dest);

  router_id_t root;

  std::vector<NetworkRoute> routes;
  std::set<router_id_t> nodes;
  std::map<router_id_t, int> shortestPaths;
  std::map<router_id_t, NetworkRoute> routingTable;
};