#pragma once

#include <memory>

class AudioWave;

class AudioInputInterface
{
public:

    virtual void process( const AudioWave & input ) = 0;
};
