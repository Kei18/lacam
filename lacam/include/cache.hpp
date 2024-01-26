// Cache definition
// Author: Zhenghong Yu

#pragma once

#include "utils.hpp"
#include <cassert>

struct Cache {
    Vertices node_cargo;
    Vertices node_id;
    std::vector<int> LRU;
    std::vector<uint> bit_lock;
    uint LRU_cnt = 0;

    Cache();
    ~Cache();

    /**
     * @brief Get the index of a specified cache block.
     * @param block A pointer to the Vertex representing the block.
     * @return The index of the cache block.
     */
    int get_cache_block_in_cache_index(Vertex* block);

    /**
     * @brief Check if a specific cargo is cached.
     * @param cargo A pointer to the Vertex representing the cargo in cache.
     * @return The index of the cached cargo, or -1 if not cached.
     */
    int get_cargo_in_cache_index(Vertex* cargo);

    /**
     * @brief Attempt to find a cached cargo and retrieve associated goals.
     * @param cargo A pointer to the Vertex representing the cargo.
     * @return A pointer to the Vertex of the cargo in the cache,
     *         or to the Vertex in the warehouse if not cached.
     */
    Vertex* try_cache_cargo(Vertex* cargo);

    /**
     * @brief Find a cache block for cargo that is not cached (cache-miss) and set goals.
     *        This is triggered when a cargo cache miss occurs.
     * @param cargo A pointer to the Vertex representing the cargo.
     * @param unloading_port A pointer to the Vertex representing the unloading port.
     * @return A pointer to the Vertex representing the cache block, or
     *         the unloading port Vertex if a suitable block cannot be found or
     *         if the cargo is already cached (in a multi-agent context).
     */
    Vertex* try_insert_cache(Vertex* cargo, Vertex* unloading_port);

    /**
     * @brief Insert cargo into cache. This occurs when an agent brings a
     *        cache-miss cargo back to the cache.
     * @param cargo A pointer to the Vertex representing the cargo.
     * @param cache_node A pointer to the Vertex representing the cache goal.
     * @return true if successful, false otherwise.
     */
    bool update_cargo_into_cache(Vertex* cargo, Vertex* cache_node);

    /**
     * @brief Release lock when retrieving a cached cargo.
     *        This occurs when an agent reaches the cached cargo.
     * @param cargo A pointer to the Vertex representing the cargo.
     * @param cache_node A pointer to the Vertex representing the cache goal.
     * @return true if successful, false otherwise.
     */
    bool update_cargo_from_cache(Vertex* cargo, Vertex* cache_node);

};