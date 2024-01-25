// Cache implementation
// Author: Zhenghong Yu

#include "cache.hpp"

Cache::Cache() {};
Cache::~Cache() {};

int Cache::get_cache_index(Vertex* cargo) {
    int cache_index = -1;

    for (int i = 0; i < node_cargo.size(); i++) {
        if (node_cargo[i] == cargo) {
            std::cerr << "Cache hit for cargo " << cargo << std::endl;
            cache_index = i;
            break;
        }
    }

    return cache_index;
}

Vertex* Cache::get_cache_cargo(Vertex* cargo) {
    int cache_index = get_cache_index(cargo);

    // If we can find cargo cached, we go to cache and get it
    if (cache_index != -1) {
        std::cerr << "Agent will go " << node_cargo[cache_index] << " to get cargo " << cargo << std::endl;
        // For here, we allow multiple agents lock on this position
        // It is impossible that a coming agent move cargo to this 
        // position while the cargo has already here
        bit_lock[cache_index] += 1;
        // We also update LRU here, since we do not want the cached cargo 
        // been evicted while the agent moving to the cache
        LRU[cache_index] = LRU_cnt++;
        return node_cargo[cache_index];
    }

    // If we cannot find cargo cached, we directly go to warehouse
    std::cerr << "Cache miss! Agent will directly to get cargo " << cargo << std::endl;
    return cargo;
}

Vertex* Cache::try_insert_cache(Vertex* cargo, Vertex* unloading_port) {
    // First, if cargo has already cached, we directly go
    // to unloading port
    if (get_cache_index(cargo) != -1) return unloading_port;

    // Second try to find a empty position to insert cargo
    // TODO: optimization, can set a flag to skip this
    for (int i = 0; i < LRU.size(); i++) {
        if (LRU[i] == 0) {
            // We reserve this position and update LRU info
            bit_lock[i] += 1;
            LRU[i] == LRU_cnt++;
            return node_cargo[i];
        }
    }

    // Third, try to find a LRU position that is not locked
    int min_value = -1;
    int min_index = -1;

    for (int i = 0; i < LRU.size(); i++) {
        // If it's not locked and (it's the first element or the smallest so far)
        if (bit_lock[i] == 0 && (min_value == -1 || LRU[i] < min_value)) {
            min_value = LRU[i];
            min_index = i;
        }
    }

    // If we can find one, return the posititon
    if (min_index != -1) {
        // We reserve this position and update LRU info
        bit_lock[min_index] += 1;
        LRU[min_index] = LRU_cnt++;
        return node_cargo[min_index];
    }
    // Else we can not insert into cache
    else return unloading_port;
}

bool Cache::update_cargo_into_cache(Vertex* cargo, Vertex* cache_node) {
    int cargo_index = get_cache_index(cargo);
    int cache_index = get_cache_index(cache_node);
    // We should only update it while it is not in cache
    assert(cargo_index == -1);
    assert(cache_index != -1);

    // Update cache
    std::cerr << "Update cargo " << cargo << " to cache " << cache_node << std::endl;
    node_cargo[cache_index] = cargo;
    // Release lock
    bit_lock[cache_index] -= 1;

    return true;
}

bool Cache::update_cargo_from_cache(Vertex* cargo, Vertex* cache_node) {
    int cargo_index = get_cache_index(cargo);
    int cache_index = get_cache_index(cache_node);
    assert(cargo_index != -1);
    assert(cache_index != -1);

    // Simply release lock
    std::cerr << "Agents gets " << cargo << "from cache " << cache_node << std::endl;
    bit_lock[cache_index] -= 1;

    return true;
}

