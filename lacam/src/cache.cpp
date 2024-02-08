// Cache implementation
// Author: Zhenghong Yu

#include "../include/cache.hpp"

Cache::Cache(std::shared_ptr<spdlog::logger> _logger, CacheType _cache_type) :
    logger(std::move(_logger)), cache_type(_cache_type) {};

Cache::~Cache() {};

bool Cache::_update_cache_evited_policy_statistics(uint index, bool fifo_option) {
    switch (cache_type) {
    case CacheType::LRU:
        LRU_cnt += 1;
        LRU[index] = LRU_cnt;
        break;
    case CacheType::FIFO:
        if (fifo_option) {
            FIFO_cnt += 1;
            FIFO[index] = FIFO_cnt;
        }
        break;
    case CacheType::RANDOM:
        break;
    default:
        logger->error("Unreachable cache state!");
        exit(1);
    }
}

int Cache::_get_cache_block_in_cache_index(Vertex* block) {
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

int Cache::_get_cargo_in_cache_index(Vertex* cargo) {
    int cargo_index = -1;
    for (uint i = 0; i < node_cargo.size(); i++) {
        if (node_cargo[i] == cargo) {
            cargo_index = i;
            break;
        }
    }
    return cargo_index;
}

bool Cache::_is_cargo_in_coming_cache(Vertex* cargo) {
    for (uint i = 0; i < node_coming_cargo.size(); i++) {
        if (node_coming_cargo[i] == cargo) {
            return true;
        }
    }
    return false;
}

Vertex* Cache::try_cache_cargo(Vertex* cargo) {
    int cache_index = _get_cargo_in_cache_index(cargo);

    // If we can find cargo cached and is not reserved to be replaced , we go to cache and get it
    if (cache_index != -1 && bit_cache_insert_lock[cache_index] == 0) {
        logger->debug("Cache hit! Agent will go {} to get cargo {}", *node_id[cache_index], *cargo);
        // For here, we allow multiple agents lock on cache get position
        // It is impossible that a coming agent move cargo to this 
        // position while the cargo has already here
        bit_cache_get_lock[cache_index] += 1;
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
    // First, if cargo has already cached or is coming on the way, we directly go
    // to unloading port
    if (_get_cargo_in_cache_index(cargo) != -1 || _is_cargo_in_coming_cache(cargo)) return unloading_port;

    // Second try to find a empty position to insert cargo
    // TODO: optimization, can set a flag to skip this
    for (uint i = 0; i < LRU.size(); i++) {
        if (LRU[i] == 0) {
            logger->debug("Find an empty cache block with index {} {}", i, *node_id[i]);
            // We lock this position and update LRU info
            bit_cache_insert_lock[i] += 1;
            // Updating coming cargo info
            node_coming_cargo[i] = cargo;
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
        if (bit_cache_insert_lock[i] == 0 && bit_cache_get_lock[i] == 0 && (min_value == -1 || LRU[i] < min_value)) {
            min_value = LRU[i];
            min_index = i;
        }
    }

    // If we can find one, return the posititon
    if (min_index != -1) {
        // We lock this position and update LRU info
        bit_cache_insert_lock[min_index] += 1;
        // Updating coming cargo info
        node_coming_cargo[min_index] = cargo;
        LRU_cnt += 1;
        LRU[min_index] = LRU_cnt;
        return node_id[min_index];
    }

    // Else we can not insert into cache
    else return unloading_port;
}

bool Cache::update_cargo_into_cache(Vertex* cargo, Vertex* cache_node) {
    int cargo_index = _get_cargo_in_cache_index(cargo);
    int cache_index = _get_cache_block_in_cache_index(cache_node);
    // We should only update it while it is not in cache
    assert(cargo_index == -1);
    assert(cache_index != -1);
    assert(_is_cargo_in_coming_cache(cargo));

    // Update cache
    logger->debug("Update cargo {} to cache block {}", *cargo, *cache_node);
    node_cargo[cache_index] = cargo;
    bit_cache_insert_lock[cache_index] -= 1;
    return true;

}

bool Cache::update_cargo_from_cache(Vertex* cargo, Vertex* cache_node) {
    int cargo_index = _get_cargo_in_cache_index(cargo);
    int cache_index = _get_cache_block_in_cache_index(cache_node);
    // We must make sure the cargo is still in the cache
    assert(cargo_index != -1);
    assert(cache_index != -1);

    // Simply release lock
    logger->debug("Agents gets {} from cache {}", *cargo, *cache_node);
    bit_cache_get_lock[cache_index] -= 1;

    return true;
}

