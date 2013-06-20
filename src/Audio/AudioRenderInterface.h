#pragma once

#include <memory>

class AudioWave;

class AudioRenderInterface
{
public:

    virtual void render( AudioWave & output ) = 0;
};
