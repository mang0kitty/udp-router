#include "network_table.h"
#include <climits>
#include <iostream>
#include <sstream>

NetworkTable::NetworkTable(router_id_t root)
{
    this->root = root;
    this->nodes.insert(root);
    this->shortestPaths[root] = 0;
}

int NetworkTable::port_to(router_id_t dest)
{
    for (auto &&route : this->routes)
    {
        if (route.source == this->root && route.dest == dest)
            return route.port;
    }

    return -1;
}

std::vector<NetworkRoute> NetworkTable::neighbours()
{
    std::vector<NetworkRoute> neighboursList;
    for (auto &&route : routes)
    {
        if (route.source != this->root)
        {
            continue;
        }

        neighboursList.push_back(route);
    }
    return neighboursList;
}

std::vector<NetworkRoute> NetworkTable::full_table()
{
    std::vector<NetworkRoute> routingTableList;

    for (auto &&entry : routingTable)
    {
        routingTableList.push_back(entry.second);
    }

    return routingTableList;
}

void NetworkTable::print()
{
    std::cout << "Routing Table:" << std::endl;

    std::cout << this->to_string();
}

std::string NetworkTable::to_string()
{
    std::stringstream out;

    out << "SOURCE   DEST    VIA-PORT    COST" << std::endl;

    for (auto &&entry : routingTable)
    {
        auto route = entry.second;
        if (route.source == this->root)
        {
            out << "  " << route.source << "       " << route.dest << "        " << route.port << "      " << route.cost << std::endl;
        }
    }

    out << std::endl;

    return out.str();
}

bool NetworkTable::update(const NetworkRoute *info)
{
    auto isUpdate = false;

    for (int i = 0; i < this->routes.size(); i++)
    {
        auto route = routes[i];

        // Filter out routes which don't match our new one
        if (route.source != info->source || route.dest != info->dest)
            continue;

        if (info->cost == route.cost && info->port == route.port)
        {
            // We're already running this route, nothing has changed
            return false;
        }

        routes[i].port = info->port;
        routes[i].cost = info->cost;
        isUpdate = true;
        break;
    }

    if (!isUpdate)
    {
        this->routes.push_back(*info);
        this->nodes.insert(info->source);
        this->nodes.insert(info->dest);
    }

    // Bellman-Ford algorithm to find shortest path through the graph
    // from root to each node.
    for (auto &&node : this->nodes)
    {
        for (auto &&route : this->routes)
        {
            int estCost = cost(route.source);
            if (estCost < INT_MAX)
                estCost += route.cost;

            if (estCost < cost(route.dest))
            {
                this->shortestPaths[route.dest] = estCost;
            }
        }
    }

    for (auto &&route : this->routes)
    {
        auto sourceCost = cost(route.source);
        if (sourceCost < INT_MAX)
        {
            auto destCost = cost(route.dest);
            if (sourceCost + route.cost < destCost)
                throw "negative weight cycle detected in graph";
        }
    }

    // Enter the paths into the routing table
    for (auto &&node : this->nodes)
    {
        if (node == this->root)
            continue;
        auto route = this->backtrack(node);
        if (route.cost == INT_MAX)
            continue;

        this->routingTable[node] = route;
    }

    return true;
}

int NetworkTable::cost(router_id_t dest)
{
    auto costEntry = this->shortestPaths.find(dest);
    if (costEntry == this->shortestPaths.end())
        return INT_MAX;

    return costEntry->second;
}

NetworkRoute NetworkTable::route(router_id_t dest)
{
    auto routeEntry = routingTable.find(dest);
    if (routeEntry == routingTable.end())
    {
        NetworkRoute result;
        result.source = this->root;
        result.dest = dest;
        result.cost = INT_MAX;
        result.port = -1;
        return result;
    }

    return routeEntry->second;
}

NetworkRoute NetworkTable::backtrack(router_id_t dest)
{
    for (auto &&route : this->routes)
    {
        // Filter out routes which don't end at our destination
        if (route.dest != dest)
            continue;

        auto sourceCost = cost(route.source);

        // If we can't reach the source, ignore the route
        if (sourceCost == INT_MAX)
            continue;

        // Filter out longer paths
        if (sourceCost + route.cost != cost(dest))
            continue;

        if (route.source == this->root)
            return route;

        auto result = backtrack(route.source);
        result.dest = dest;
        result.cost += route.cost;
        return result;
    }

    // Not found case
    NetworkRoute result;
    result.source = this->root;
    result.dest = dest;
    result.cost = INT_MAX;
    result.port = -1;
    return result;
}