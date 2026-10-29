#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set> // Added for lightning-fast disk index tracking
#include <mutex>
#include <cmath>

enum class BlockMaterial : uint8_t { 
    MAT_AIR, 
    MAT_STONE, 
    MAT_GRASS 
};

//---------------
// Config
//---------------
constexpr int CHUNK_SIZE_X   = 48;
constexpr int CHUNK_SIZE_Z   = 48;
constexpr int NORMAL_HEIGHT  = 200;
constexpr int MAX_HEIGHT     = 400;

//---------------
// Block
//---------------
struct Block {
    BlockMaterial material{}; 
    uint16_t durability = 0;
    uint16_t temp       = 0;
    uint16_t wetness    = 0;
    uint16_t charge     = 0;

    bool operator==(const Block& other) const {
        return material == other.material &&
               durability == other.durability &&
               temp == other.temp &&
               wetness == other.wetness &&
               charge == other.charge;
    }
};

//---------------
// Chunk
//---------------
struct Chunk {
    int32_t chunk_x = 0;
    int32_t chunk_z = 0;

    int min_y  = -100;
    int height = NORMAL_HEIGHT;

    std::vector<Block> blocks;

    bool dirty    = false;
    bool sleeping = true;
    bool loaded   = false;

    void allocate() {
        blocks.resize(static_cast<size_t>(height) * CHUNK_SIZE_X * CHUNK_SIZE_Z);
        loaded = true;
        dirty  = true;
    }

    void free_memory() {
        blocks.clear();
        blocks.shrink_to_fit();
        loaded = false;
    }

    Block& at(int x, int y, int z) {
        const int local_y = y - min_y;
        const size_t idx = (static_cast<size_t>(local_y) * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;
        return blocks[idx];
    }

    const Block& at(int x, int y, int z) const {
        const int local_y = y - min_y;
        const size_t idx = (static_cast<size_t>(local_y) * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;
        return blocks[idx];
    }

    void set_block(int x, int y, int z, const Block& b) {
        at(x, y, z) = b;
        dirty = true;
    }
};

//---------------
// World
//---------------
class World {
public:
    std::unordered_map<uint64_t, Chunk> chunks;
    std::unordered_set<uint64_t> saved_chunks_index; // Prevents slow file scanning crashes
    std::mutex chunks_mutex;
    std::string save_filepath = "compressed_world.dat";

    World() {
        build_disk_index(); // Automatically indexes your save file on startup
    }

    static uint64_t pack_chunk_coord(int32_t cx, int32_t cz) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(cx)) << 32) |
                static_cast<uint32_t>(cz);
    }

    //----- Procedural Generation Rule System -----
    void procedurally_generate_chunk(Chunk& c) {
        Block grass{ BlockMaterial::MAT_GRASS, 100 };
        Block stone{ BlockMaterial::MAT_STONE, 250 };
        Block air{ BlockMaterial::MAT_AIR, 0 };

        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                for (int y = c.min_y; y < c.min_y + c.height; ++y) {
                    if (y < 0) {
                        c.at(x, y, z) = stone;
                    } else if (y == 0) {
                        c.at(x, y, z) = grass;
                    } else {
                        c.at(x, y, z) = air;
                    }
                }
            }
        }
        c.dirty = false;
    }

    //----- Streaming Load/Unload On-Demand Engine -----
    Chunk* get_or_generate_chunk(int32_t cx, int32_t cz) {
        std::lock_guard lock(chunks_mutex);
        uint64_t key = pack_chunk_coord(cx, cz);
        
        auto it = chunks.find(key);
        if (it != chunks.end()) {
            return &it->second;
        }

        Chunk c;
        c.chunk_x = cx;
        c.chunk_z = cz;
        c.min_y   = -100;
        c.height  = NORMAL_HEIGHT;
        c.allocate();

        // Instant look-up avoids linear file scanning
        if (saved_chunks_index.find(key) != saved_chunks_index.end()) {
            if (load_single_chunk_from_disk(cx, cz, c)) {
                auto [new_it, _] = chunks.emplace(key, std::move(c));
                return &new_it->second;
            }
        }

        procedurally_generate_chunk(c);
        auto [new_it, _] = chunks.emplace(key, std::move(c));
        return &new_it->second;
    }

    //----- Hardware RAM Stress Tester -----
    void fill_ram_until_crash() {
        std::cout << "====================================================\n";
        std::cout << "[CRASH TEST] BEGINNING MEMORY ALLOCATION STRESS TEST\n";
        std::cout << "Generating chunks continuously to force out-of-memory...\n";
        std::cout << "====================================================\n\n";

        uint64_t total_chunks_allocated = 0;
        int layer_radius = 1;

        try {
            while (true) {
                for (int dz = -layer_radius; dz <= layer_radius; ++dz) {
                    for (int dx = -layer_radius; dx <= layer_radius; ++dx) {
                        if (std::abs(dx) == layer_radius || std::abs(dz) == layer_radius) {
                            
                            get_or_generate_chunk(dx, dz);
                            total_chunks_allocated++;

                            if (total_chunks_allocated % 5000 == 0) {
                                double gigabytes_estimated = (total_chunks_allocated * 460800 * sizeof(Block)) / (1024.0 * 1024.0 * 1024.0);
                                std::cout << "[Allocated] Chunks: " << total_chunks_allocated 
                                          << " | Map Radius: " << layer_radius 
                                          << " | Approx Raw Voxel RAM: " << gigabytes_estimated << " GB\n";
                            }
                        }
                    }
                }
                layer_radius++;
            }
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "\n[CRASH SUCCESS] Caught std::bad_alloc! Out of Memory exception reached.\n";
            std::cerr << "Total Chunks Loaded into RAM: " << total_chunks_allocated << "\n";
            exit(1);
        }
    }

    void unload_distant_chunks(int32_t player_cx, int32_t player_cz, int keep_radius) {
        std::lock_guard lock(chunks_mutex);
        auto it = chunks.begin();
        while (it != chunks.end()) {
            Chunk& c = it->second;
            if (std::abs(c.chunk_x - player_cx) > keep_radius || std::abs(c.chunk_z - player_cz) > keep_radius) {
                if (c.dirty) {
                    save_single_chunk_to_disk(c);
                }
                c.free_memory();
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void set_block_world(int wx, int wy, int wz, const Block& b) {
        int cx = wx / CHUNK_SIZE_X;
        int cz = wz / CHUNK_SIZE_Z;
        int lx = wx % CHUNK_SIZE_X;
        int lz = wz % CHUNK_SIZE_Z;
        if (lx < 0) { lx += CHUNK_SIZE_X; --cx; }
        if (lz < 0) { lz += CHUNK_SIZE_Z; --cz; }

        Chunk* chunk = get_or_generate_chunk(cx, cz);
        chunk->set_block(lx, wy, lz, b);
        chunk->dirty = true;
    }

    Block* get_block_world(int wx, int wy, int wz) {
        int cx = wx / CHUNK_SIZE_X;
        int cz = wz / CHUNK_SIZE_Z;
        int lx = wx % CHUNK_SIZE_X;
        int lz = wz % CHUNK_SIZE_Z;
        if (lx < 0) { lx += CHUNK_SIZE_X; --cx; }
        if (lz < 0) { lz += CHUNK_SIZE_Z; --cz; }

        Chunk* chunk = get_or_generate_chunk(cx, cz);
        return &chunk->at(lx, wy, lz);
    }

private:
    // Builds a fast index layout map of what chunks reside on disk
    void build_disk_index() {
        std::ifstream in(save_filepath, std::ios::binary);
        if (!in.is_open()) return;

        uint64_t read_key = 0;
        while (in.read(reinterpret_cast<char*>(&read_key), sizeof(read_key))) {
            int32_t rx, rz, rmin_y, rheight;
            in.read(reinterpret_cast<char*>(&rx), sizeof(rx));
            in.read(reinterpret_cast<char*>(&rz), sizeof(rz));
            in.read(reinterpret_cast<char*>(&rmin_y), sizeof(rmin_y));
            in.read(reinterpret_cast<char*>(&rheight), sizeof(rheight));

            uint64_t rle_count = 0;
            in.read(reinterpret_cast<char*>(&rle_count), sizeof(rle_count));

            saved_chunks_index.insert(read_key);
            in.seekg(rle_count * (sizeof(uint32_t) + sizeof(Block)), std::ios::cur);
        }
    }

    void save_single_chunk_to_disk(const Chunk& chunk) {
        std::ofstream out(save_filepath, std::ios::binary | std::ios::app);
        if (!out.is_open()) return;

        uint64_t key = pack_chunk_coord(chunk.chunk_x, chunk.chunk_z);
        out.write(reinterpret_cast<const char*>(&key), sizeof(key));
        out.write(reinterpret_cast<const char*>(&chunk.chunk_x), sizeof(chunk.chunk_x));
        out.write(reinterpret_cast<const char*>(&chunk.chunk_z), sizeof(chunk.chunk_z));
        out.write(reinterpret_cast<const char*>(&chunk.min_y), sizeof(chunk.min_y));
        out.write(reinterpret_cast<const char*>(&chunk.height), sizeof(chunk.height));

        std::vector<std::pair<uint32_t, Block>> compressed;
        if (!chunk.blocks.empty()) {
            Block current_run_block = chunk.blocks[0];
            uint32_t current_run_count = 1;

            for (size_t i = 1; i < chunk.blocks.size(); ++i) {
                if (chunk.blocks[i] == current_run_block) {
                    current_run_count++;
                } else {
                    compressed.push_back({current_run_count, current_run_block});
                    current_run_block = chunk.blocks[i];
                    current_run_count = 1;
                }
            }
            compressed.push_back({current_run_count, current_run_block});
        }

        uint64_t rle_elements_count = compressed.size();
out.write(reinterpret_cast<const char*>(&rle_elements_count), sizeof(rle_elements_count));
for (const auto& run : compressed) {
out.write(reinterpret_cast<const char*>(&run.first), sizeof(run.first));
out.write(reinterpret_cast<const char*>(&run.second), sizeof(run.second));
}
saved_chunks_index.insert(key);
}
bool load_single_chunk_from_disk(int32_t cx, int32_t cz, Chunk& out_chunk) {
std::ifstream in(save_filepath, std::ios::binary);
if (!in.is_open()) return false;
uint64_t target_key = pack_chunk_coord(cx, cz);
uint64_t read_key = 0;
while (in.read(reinterpret_cast<char*>(&read_key), sizeof(read_key))) {
int32_t rx, rz, rmin_y, rheight;
in.read(reinterpret_cast<char*>(&rx), sizeof(rx));
in.read(reinterpret_cast<char*>(&rz), sizeof(rz));
in.read(reinterpret_cast<char*>(&rmin_y), sizeof(rmin_y));
in.read(reinterpret_cast<char*>(&rheight), sizeof(rheight));
uint64_t rle_count = 0;
in.read(reinterpret_cast<char*>(&rle_count), sizeof(rle_count));
if (read_key == target_key) {
out_chunk.chunk_x = rx;
out_chunk.chunk_z = rz;
out_chunk.min_y = rmin_y;
out_chunk.height = rheight;
out_chunk.blocks.clear();
for (uint64_t i = 0; i < rle_count; ++i) {
uint32_t run_length = 0;
Block run_block;
in.read(reinterpret_cast<char*>(&run_length), sizeof(run_length));
in.read(reinterpret_cast<char*>(&run_block), sizeof(run_block));
out_chunk.blocks.insert(out_chunk.blocks.end(), run_length, run_block);
}
out_chunk.loaded = true;
out_chunk.dirty = false;
return true;
} else {
in.seekg(rle_count * (sizeof(uint32_t) + sizeof(Block)), std::ios::cur);
}
}
return false;
}
};