#include "npc_data.h"


#include <algorithm>

std::string get_race_name(
    uint32_t race_id,
    const std::vector<race_definition>& race_defs)
{
    for (const auto& race : race_defs)
    {
        if (race.id == race_id)
        {
            return race.name;
        }
    }

    return "unknown";
}

std::string generate_race_name(
    const race_data& race,
    const std::vector<race_definition>& race_defs)
{
    if (race.ancestry.empty())
    {
        return "unknown";
    }

    std::vector<race_piece> sorted = race.ancestry;

    std::sort(
        sorted.begin(),
        sorted.end(),
        [](const race_piece& a,
        const race_piece& b)
        {
        return a.percent > b.percent;
        });

    const race_piece& primary = sorted[0];

    if (primary.percent >= 99.9f)
    {
        return "pure " +
               get_race_name(
                   primary.race_id,
                   race_defs);
    }

    if (sorted.size() >= 2)
    {
        const race_piece& secondary = sorted[1];

        bool primary_half =
            primary.percent >= 49.0f &&
            primary.percent <= 51.0f;

        bool secondary_half =
            secondary.percent >= 49.0f &&
            secondary.percent <= 51.0f;

        if (primary_half && secondary_half)
        {
            return "half " +
                   get_race_name(
                       primary.race_id,
                       race_defs) +
                   " half " +
                   get_race_name(
                       secondary.race_id,
                       race_defs);
        }
    }

    return get_race_name(
               primary.race_id,
               race_defs)
           + " mixed blood";
}