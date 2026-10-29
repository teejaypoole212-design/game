#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "npc_data.h"

struct npc {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;

    std::vector<float> vel_x;
    std::vector<float> vel_y;
    std::vector<float> vel_z;
    //temp note out until systems set up
    std::vector<Religion> religion; //religion can change
    std::vector<Job> job; //job can change too
    std::vector<std::vector <Job>> past_jobs;
    std::vector<std::vector<Skill>> skills; //every skill they have skill_list will be indeividual list of skills that can be read off of when needed (both magic and meele)


    std::vector<uint32_t> generation;
    std::vector<uint32_t> id;
    std::vector<std::string> name;
    std::vector<Gender> gender; //might bring minor effects to stats, personality and even actions
    std::vector<race_data> race;
    std::vector<Emotion> current_emotion;
    std::vector<Emotion> past_emotion; //both gathered from avrage emotions felt at that time and will impact lots of things 
    std::vector<int> age;
    std::vector<uint8_t> is_alive;
    std::vector<float> hight; //will determine reach, strength, other minor stats (in inches)
};



