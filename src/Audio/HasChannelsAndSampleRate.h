#pragma once

class HasChannelsAndSampleRate
{
public:

    virtual unsigned sample_rate() const = 0;
    virtual unsigned channels() const = 0;
};
