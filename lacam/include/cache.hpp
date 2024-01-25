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
    int get_cache_index(Vertex* cargo);
    Vertex* get_cache_cargo(Vertex* cargo);
    Vertex* try_insert_cache(Vertex* cargo, Vertex* unloading_port);
    bool update_cargo_into_cache(Vertex* cargo, Vertex* cache_node);
    bool update_cargo_from_cache(Vertex* cargo, Vertex* cache_node);
};