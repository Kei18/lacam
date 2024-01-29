// Cache implementation
// Author: Zhenghong Yu

#include "../include/cache.hpp"

Cache::Cache(std::shared_ptr<spdlog::logger> _logger) : logger(std::move(_logger)) {};
Cache::~Cache() {};

int Cache::get_cache_block_in_cache_index(Vertex* block) {
    int cache_index = -1;
    for (uint i = 0; i < node_id.size(); i++) {
        if (node_id[i] == block) {
            cache_index = i;
            break;
        }
    }
    // Cache goals must in cache
    assert(cache_index != -1);
    return cache_index;
}

int Cache::get_cargo_in_cache_index(Vertex* cargo) {
    int cargo_index = -1;
    for (uint i = 0; i < node_cargo.size(); i++) {
        if (node_cargo[i] == cargo) {
            cargo_index = i;
            break;
        }
    }
    return cargo_index;
}

Vertex* Cache::try_cache_cargo(Vertex* cargo) {
    int cache_index = get_cargo_in_cache_index(cargo);

    // If we can find cargo cached, we go to cache and get it
    if (cache_index != -1) {
        logger->debug("Cache hit! Agent will go {} to get cargo {}", *node_id[cache_index], *cargo);
        // For here, we allow multiple agents lock on this position
        // It is impossible that a coming agent move cargo to this 
        // position while the cargo has already here
        bit_lock[cache_index] += 1;
        // We also update LRU here, since we do not want the cached cargo 
        // been evicted while the agent moving to the cache
        LRU_cnt += 1;
        LRU[cache_index] = LRU_cnt;
        return node_id[cache_index];
    }

    // If we cannot find cargo cached, we directly go to warehouse
    logger->debug("Cache miss! Agent will directly to get cargo {}", *cargo);
    return cargo;
}

Vertex* Cache::try_insert_cache(Vertex* cargo, Vertex* unloading_port) {
    // First, if cargo has already cached, we directly go
    // to unloading port
    if (get_cargo_in_cache_index(cargo) != -1) return unloading_port;

    // Second try to find a empty position to insert cargo
    // TODO: optimization, can set a flag to skip this
    for (uint i = 0; i < LRU.size(); i++) {
        if (LRU[i] == 0) {
            logger->debug("Find an empty cache block with index {} {}", i, *node_id[i]);
            // We reserve this position and update LRU info
            LRU_cnt += 1;
            LRU[i] = LRU_cnt;
            return node_id[i];
        }
    }

    // Third, try to find a LRU position that is not locked
    int min_value = -1;
    int min_index = -1;

    for (uint i = 0; i < LRU.size(); i++) {
        // If it's not locked and (it's the first element or the smallest so far)
        if (bit_lock[i] == 0 && (min_value == -1 || LRU[i] < min_value)) {
            min_value = LRU[i];
            min_index = i;
        }
    }

    // If we can find one, return the posititon
    if (min_index != -1) {
        // We reserve this position and update LRU info
        LRU_cnt += 1;
        LRU[min_index] = LRU_cnt;
        return node_id[min_index];
    }
    // Else we can not insert into cache
    else return unloading_port;
}

bool Cache::update_cargo_into_cache(Vertex* cargo, Vertex* cache_node) {
    int cargo_index = get_cargo_in_cache_index(cargo);
    int cache_index = get_cache_block_in_cache_index(cache_node);
    // We should only update it while it is not in cache
    assert(cargo_index == -1);

    // Update cache
    logger->debug("Update cargo {} to cache block {}", *cargo, *cache_node);
    node_cargo[cache_index] = cargo;

    return true;
}

bool Cache::update_cargo_from_cache(Vertex* cargo, Vertex* cache_node) {
    int cargo_index = get_cargo_in_cache_index(cargo);
    int cache_index = get_cache_block_in_cache_index(cache_node);
    assert(cargo_index != -1);
    assert(cache_index != -1);

    // Simply release lock
    logger->debug("Agents gets {} from cache {}", *cargo, *cache_node);
    bit_lock[cache_index] -= 1;

    return true;
}

