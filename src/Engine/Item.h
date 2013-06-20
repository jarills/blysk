#pragma once

#include <vector>
#include <utility>

class Item
{
public:

    // set of warp-points: relative time (in seconds), position in sample
    typedef std::vector< std::pair< double, unsigned int > > warp_vector;

    // set of gain points: relative time (in seconds), gain
    typedef std::vector< std::pair< double, float > > gain_vector;

private:

    warp_vector warp_;
    gain_vector gain_;

    double start_pos_; // seconds
    double end_pos_; // seconds

};
