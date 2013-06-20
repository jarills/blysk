#pragma once

#include "HasChannelsAndSampleRate.h"

#include <memory>

class RtAudio;

class AudioRenderInterface;
class AudioInputInterface;

class AudioDevice
{
public:
    AudioDevice( unsigned sample_rate, unsigned buffer_size );

    ~AudioDevice();

    void start( unsigned inputs, unsigned outputs );
    void set_output_handler( std::shared_ptr< AudioRenderInterface > handler );
    void set_input_handler( std::shared_ptr< AudioInputInterface > handler );

    unsigned sample_rate() const;
    unsigned buffer_size() const;

    unsigned input_channels() const;
    unsigned output_channels() const;

    std::shared_ptr< AudioRenderInterface > output_handler();
    std::shared_ptr< AudioInputInterface > input_handler();

private:

    std::shared_ptr< AudioRenderInterface > output_handler_;
    std::shared_ptr< AudioInputInterface > input_handler_;

    unsigned sample_rate_;
    unsigned buffer_size_;

    std::shared_ptr< RtAudio > rt_audio_;

    unsigned inputs_;
    unsigned outputs_;

};
