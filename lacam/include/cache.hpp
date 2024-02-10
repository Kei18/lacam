// Cache definition
// Author: Zhenghong Yu

#pragma once

#include "utils.hpp"
#include <cassert>

struct Cache {
    Vertices node_cargo;
    Vertices node_id;
    Vertices node_coming_cargo;
    std::vector<uint> bit_cache_get_lock;
    std::vector<uint> bit_cache_insert_lock;
    std::shared_ptr<spdlog::logger> logger;
    CacheType cache_type;
    std::mt19937* randomSeed;

    // LRU paras
    std::vector<int> LRU;
    uint LRU_cnt = 0;

    // FIFO paras
    std::vector<int> FIFO;
    uint FIFO_cnt = 0;

    // Random paras (no paras)

    Cache(std::shared_ptr<spdlog::logger> _logger, CacheType _cache_type, std::mt19937* _randomSeed = 0);
    ~Cache();

    /**
     * @brief Update evicted policy statistics
     * @param lru_index An index used for lru policy
     * @param fifo_option An option to control fifo policy
     * @return true if successful, false otherwise
    */
    bool _update_cache_evited_policy_statistics(uint index, bool fifo_option);

    /**
     * @brief Get index of evicted cache block
     * @param
     * @return true if successful, false otherwise
    */
    int _get_cache_evited_policy_index();

    /**
     * @brief Get the index of a specified cache block.
     * @param block A pointer to the Vertex representing the block.
     * @return The index of the cache block.
     */
    int _get_cache_block_in_cache_index(Vertex* block);

    /**
     * @brief Check if a specific cargo is cached.
     * @param cargo A pointer to the Vertex representing the cargo.
     * @return The index of the cached cargo, or -1 if not cached.
     */
    int _get_cargo_in_cache_index(Vertex* cargo);

    /**
     * @brief Check if a specific cargo is coming to cache
     * @param cargo A pointer to the Vertex representing the cargo.
     * @return true if is coming to cacehe, or false.
    */

    bool _is_cargo_in_coming_cache(Vertex* cargo);

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