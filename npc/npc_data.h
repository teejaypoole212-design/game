#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "skill.h"

enum class Emotion {
    HAPPY,
    SAD,
    MAD,
    SCARED,
    SUPRISED,
    DISGUSTED,
    CONTEMPT //basic emotions
};

//----------------------------------------

struct race_piece
{
    uint32_t race_id;
    float percent;
};

struct race_definition
{
    uint32_t id;
    std::string name;
};

struct race_data
{
    std::vector<race_piece> ancestry;
};

extern std::vector<race_definition> race_defs; //global list to acess all race options

std::string get_race_name(uint32_t race_id, const std::vector<race_definition>& race_defs);

std::string generate_race_name(const race_data& race, const std::vector<race_definition>& race_defs);


//----------------------------------------


enum class Gender {
    MALE,
    FEMALE,
    NA,
    BOTH
};


//---------------------------------------




//MONOTHEISM: Worship of a single, all-powerful deity.
//POLYTHEISM: Worship of a pantheon of multiple distinct gods.
//DUALISM: Belief in two opposing, equal cosmic forces.
//ANIMISM: Belief that all elements of nature possess a spiritual essence.
//NONTHEISM: Spiritual systems focused on a cosmic force or philosophy rather than personal gods.


enum class religion_type {
    MONOTHEISM,
    POLYTHEISM,
    DUALISM,
    NONTHEISM,
    ANIMISM
};

enum class restriction_type {
    DIET,
    ACTONS,
    MORALS,
    ACCEPTABLITY //some religions might prevent some for accpeting others... (racism in my game)
};

struct restriction {
    restriction_type type;
    std::string restriction; //if type food then this might be pork, or if type actions than it might be murder, casing and spelling matters 
    int severity; //how severe it is and how likely the person is to follow it
};



struct Religion {
    religion_type type;
    uint32_t id; //the gods or elemental names that they pray to is tied to its id (so if people share their religion it will have same id)
    bool has_restrictions;
    std::vector<restriction> restriction_list; //so religions can have multiple restrictions
};


//------------------------------------


enum class job_type {
    SKILLED_JOB, //highly skilled job, blacksmith carpenter etc
    UNSKILLED_JOB, //hard labor, no skill needed
    SPECIAL_JOB //king, leaders, just over all speical jobs
};

enum class job_rank {
    APPRENTICE,
    STANDARD,
    ADVANCED,
    MASTER,
    GRANDMASTER
};

struct Job {
    job_type type;
    job_rank rank;
    uint8_t level; // 1-5 within rank
    std::string name;
};



//-------------------------------
