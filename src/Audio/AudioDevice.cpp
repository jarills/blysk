#include "AudioDevice.h"

#include <vector>
#include <exception>

#include "System/RtAudio.h"

#include "Audio/AudioException.h"
#include "Audio/AudioWave.h"

#include "Audio/AudioInputInterface.h"
#include "Audio/AudioRenderInterface.h"

using namespace std;

namespace
{

int float_callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double /*streamTime*/, RtAudioStreamStatus status, void *userData )
{
    if ( status )
        std::cout << "Stream overflow detected!" << std::endl;

    if ( !userData )
    {
        cout << "!userData" << endl;
        return -1;
    }

    AudioDevice * device = static_cast< AudioDevice * >( userData );

    const float *pAudioIn = static_cast< const float * >( inputBuffer );
    float *pAudioOut = static_cast< float * >( outputBuffer );

    try
    {
        if ( device->input_channels() && !!device->input_handler() )
        {
            AudioWave input_wave( device->input_channels(), device->sample_rate() );

            for ( unsigned int i = 0; i < nBufferFrames; ++i )
            {
                for ( unsigned int c = 0; c < input_wave.channels(); ++c )
                {
                    input_wave.appendSample( c, pAudioIn[ i * 2 + c ] );
                }
            }

            device->input_handler()->process( input_wave );
        }

        if ( device->output_channels() && !!device->output_handler() )
        {
            AudioWave out( device->output_channels(), device->sample_rate() );

            out.resize(nBufferFrames, 0.0f );
            device->output_handler()->render( out );

            for ( unsigned int c = 0; c < device->output_channels(); ++c )
            {
                const std::vector< float >& out_channel = out.getSamples( c );

                for ( unsigned int i = 0; i < nBufferFrames; ++i )
                {
                    pAudioOut[ i * 2 + c ] = out_channel[ i ];
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        cout << "Exception caught: " << e.what() << endl;
        return -1;
    }

    return 0;
}
}

void AudioDevice::set_output_handler( std::shared_ptr< AudioRenderInterface > handler )
{
    output_handler_ = handler;
}

void AudioDevice::set_input_handler( std::shared_ptr< AudioInputInterface > handler )
{
    input_handler_ = handler;
}

std::shared_ptr< AudioInputInterface > AudioDevice::input_handler()
{
    return input_handler_;
}

std::shared_ptr< AudioRenderInterface > AudioDevice::output_handler()
{
    return output_handler_;
}

AudioDevice::AudioDevice( unsigned sample_rate, unsigned buffer_size )
    :
      sample_rate_(sample_rate),
      buffer_size_(buffer_size),
      rt_audio_( new RtAudio ),
      inputs_(0),
      outputs_(0)
{
}

void AudioDevice::start( unsigned inputs, unsigned outputs )
{
    if ( rt_audio_->getDeviceCount() < 1 )
    {
        throw AudioException("No audio devices found!");
    }

    // Determine the number of devices available
    //unsigned int devices = rt_audio_->getDeviceCount();

    RtAudio::StreamParameters inParams, outParams;

    inParams.deviceId = rt_audio_->getDefaultInputDevice();
    inParams.nChannels = inputs;
    inParams.firstChannel = 0;

    outParams.deviceId = rt_audio_->getDefaultOutputDevice();
    outParams.nChannels = outputs;
    outParams.firstChannel = 0;

    try {
        inputs_ = inputs;
        outputs_ = outputs;
        rt_audio_->openStream( outputs ? &outParams : NULL, inputs ? &inParams : NULL, RTAUDIO_FLOAT32, sample_rate_, &buffer_size_, &float_callback, this );
        rt_audio_->startStream();
    }
    catch ( RtError& e )
    {
        throw AudioException(e.what());
    }
}

AudioDevice::~AudioDevice()
{
    try
    {
        if ( rt_audio_->isStreamRunning() ) rt_audio_->stopStream();
        if ( rt_audio_->isStreamOpen() ) rt_audio_->closeStream();
    }
    catch (RtError& e)
    {
        throw AudioException(e.what());
    }
}

unsigned AudioDevice::sample_rate() const
{
    return sample_rate_;
}

unsigned AudioDevice::buffer_size() const
{
    return buffer_size_;
}

unsigned AudioDevice::output_channels() const
{
    return outputs_;
}

unsigned AudioDevice::input_channels() const
{
    return inputs_;
}
