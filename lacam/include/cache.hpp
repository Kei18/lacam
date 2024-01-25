// Cache definition
// Author: Zhenghong Yu

#pragma once

#include "utils.hpp"
#include "graph.hpp"
#include <cassert>

struct Cache {
    Vertices node_cargo;
    Vertices node_id;
    std::vector<int> LRU;
    std::vector<uint> bit_lock;
    uint LRU_cnt = 0;

    Cache();
    ~Cache();

    // Get cache block in cache index
    // Input: a specified block
    // Output: index
    int get_cache_block_in_cache_index(Vertex* block);

    // Check whether the specific cargo is cached
    // Input: a specified cargo
    // Output: cached cargo index, -1 if not cached
    int get_cargo_in_cache_index(Vertex* cargo);

    // Try to find the cached cargo and get goals
    // Input: a specified cargo
    // Output: cargo vertex in cache, cargo vertex in warehouse instead
    Vertex* try_cache_cargo(Vertex* cargo);

    // Try to find a cache block for cache-miss cargo, and set goals
    // this should happened when the cargo cache miss
    // Input: a specified cargo, a unloading port vertex
    // Output: cache block vertex, unloading port vertex if cannot find or already cached (multi agent)
    Vertex* try_insert_cache(Vertex* cargo, Vertex* unloading_port);

    // Try to insert cargo into cache, this should happened when an agent
    // bring a cache miss cargo back to the cache
    // Input: a specified cargo, a cache goal
    // Output: true, false if failed
    bool update_cargo_into_cache(Vertex* cargo, Vertex* cache_node);

    // Release lock when get a cached cargo, this should happened when an agent 
    // reach the cached cargo
    // Input: a specified cargo, a cache goal
    // Output: true, false if failed
    bool update_cargo_from_cache(Vertex* cargo, Vertex* cache_node);
};