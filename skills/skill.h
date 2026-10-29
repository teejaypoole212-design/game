#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class skill_type {
    MELEE,
    MAGIC,
    SPEICAL
};


struct Skill{
    skill_type type;
    std::string name;
    uint8_t level;
};