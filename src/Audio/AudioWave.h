#pragma once

#include <vector>
#include <memory>

#include "AudioException.h"
#include "HasChannelsAndSampleRate.h"

class AudioWave : public HasChannelsAndSampleRate
{
public:

	typedef std::shared_ptr< AudioWave > ptr_t;

public:

	AudioWave( unsigned int channels, unsigned sample_rate );

	explicit AudioWave( const std::string& filename );

	static ptr_t clone( AudioWave & src );

	bool saveToWav( const std::string& filename ) const;

	void appendSample( unsigned int channel, float sample );

	void appendSamples( unsigned int channel, const std::vector< float >& samples );
    void appendSamples( unsigned int channel, const std::vector< float >& samples, unsigned offset, unsigned size );

	void append( const AudioWave& other );
	void append_smoothed( const AudioWave & other, unsigned smooth_size );

	// will add "size" samples from "other", beginning at "offset" to the end of this wave
	void append( const AudioWave& other, unsigned offset, unsigned size );

	// same like append, but will stretch audio material by stretch_factor (resampling)
    void append_stretched( const AudioWave & other, unsigned offset, unsigned size, double stretch_factor, bool high_quality );

	const std::vector< float >& getSamples( unsigned int channel ) const;
	std::vector< float >& getSamples( unsigned int channel );

	unsigned int minSize() const;

	unsigned int maxSize() const;

	void mixToMono();

	void toStereo( float balance_left, float balance_right );

    void add( const AudioWave& other, unsigned dst_offset, unsigned src_offset, unsigned size, float gain = 1.0f );
    void add( const AudioWave& other, float gain = 1.0f ); // add whole wave

	unsigned int channelSize( unsigned int channel ) const;

	void scaleChannel( unsigned int channel, float factor );

	void scaleChannels( float factor );

	float maxLevel( unsigned int channel ) const;

	float maxLevel() const;

	void normalize();

	bool uniformSize() const;

	unsigned int channels() const;

	void resize( unsigned int newSize, float fillWith = 0.0f );

	void fade_out( unsigned start, unsigned size );

	unsigned sample_rate() const;

	void apply_hamming_window();
	void apply_hanning_window();
	void apply_blackman_window();

    float length() const; // in seconds

private:

	std::vector< std::vector< float > > m_Data;
	unsigned sample_rate_;

};
