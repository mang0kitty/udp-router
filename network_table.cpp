#include "network_table.h"

int NetworkTable::cost(router_id_t source, router_id_t dest)
{
    return this->pathCost.at(std::tuple(source, dest));
}

std::vector<network_table_update> NetworkTable::update(network_table_update *info)
{
    this->traversePort.insert_or_assign(std::tuple(info->source, info->dest), info->port);

    std::vector<network_table_update> pendingUpdates;

    // TODO: This needs to be a Bellman-Ford path update with the new data.
    //       It should update the entire pathCost map to reflect the changes that were made.
    //       If we improve a known shortest path, we should notify our neighbours about that
    //       by reporting it as an update (in pendingUpdates).
    this->pathCost.insert_or_assign(std::tuple(info->source, info->dest), info->cost);

    return pendingUpdates;
}