#include <iostream>

#include "npc_data.h"
#include "world.h"



int main()
{
    std::vector<race_definition> race_defs =
    {
        {0, "human"},
        {1, "elf"},
        {2, "dwarf"},
        {3, "orc"}
    };

    race_data race_1;
    race_1.ancestry =
    {
        {1, 100.0f}
    };

    race_data race_2;
    race_2.ancestry =
    {
        {1, 50.0f},
        {0, 50.0f}
    };

    race_data race_3;
    race_3.ancestry =
    {
        {1, 75.0f},
        {0, 25.0f}
    };

    race_data race_4;
    race_4.ancestry =
    {
        {1, 60.0f},
        {0, 30.0f},
        {2, 10.0f}
    };

    std::cout << generate_race_name(race_1, race_defs)
              << '\n';

    std::cout << generate_race_name(race_2, race_defs)
              << '\n';

    std::cout << generate_race_name(race_3, race_defs)
              << '\n';

    std::cout << generate_race_name(race_4, race_defs)
              << '\n';


    World world;

    std::cout << "Starting Infinite Streaming World Simulation...\n";

    // 1. Player spawns at origin block coordinates. Chunk (0,0) auto-generates.
    Block* b1 = world.get_block_world(0, 0, 0);
    std::cout << "Spawn block material type index: " << static_cast<int>(b1->material) << "\n";

    // 2. Player teleports or flies out 15 miles into the world map.
    // This dynamically handles deep block coordinates without bloating your hardware memory bounds.
    int deep_world_x = 15 * 5280; 
    world.set_block_world(deep_world_x, -5, 0, Block{BlockMaterial::MAT_STONE, 999});

    // Verify it instantly streams back safely
    Block* b2 = world.get_block_world(deep_world_x, -5, 0);
    std::cout << "Deep block modified durability: " << b2->durability << " -> OK!\n";

    // 3. Keep memory clean by trimming out blocks far from player chunk focus
    int current_player_chunk_x = deep_world_x / 48;
    world.unload_distant_chunks(current_player_chunk_x, 0, 4); 

    std::cout << "Streaming routine completed safely. Tracked memory structures minimized.\n";

    return 0;
}
