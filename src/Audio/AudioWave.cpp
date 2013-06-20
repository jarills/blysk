/*
*  AudioWave.cpp
*  viz
*
*  Created by Jaroslaw Ilski on 11.03.11.
*  Copyright 2011 __MyCompanyName__. All rights reserved.
*
*/

#include "AudioWave.h"

#include <fstream>
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <sndfile.h>
#include <cstring>
#include <functional>
#include <samplerate.h>

#include "Maths/chuck_fft.h"

using namespace std;

AudioWave::AudioWave( unsigned int channels, unsigned sample_rate )
	:
sample_rate_(sample_rate)
{
	assert( channels != 0 );
	assert( sample_rate_ != 0);

	for ( unsigned int i = 0; i < channels; ++i )
	{
		m_Data.push_back( std::vector< float >() );
	}
}

AudioWave::ptr_t AudioWave::clone( AudioWave & src )
{
	ptr_t new_clone = std::make_shared< AudioWave >( src.channels(), src.sample_rate() );
	new_clone->append( src );
	return new_clone;
}

AudioWave::AudioWave( const std::string& filename )
{
	assert( !filename.empty() );

	SF_INFO sf_info;

	SNDFILE* sf = sf_open( filename.c_str(), SFM_READ, &sf_info );

	if ( !sf )
	{
		throw AudioException( std::string("loading of '" + filename + "' failed: " + sf_strerror(sf)).c_str() );
	}

	unsigned channels = sf_info.channels;
	sample_rate_ = sf_info.samplerate;

	assert( channels != 0 );

	if ( channels == 0 )
	{
		throw AudioException( std::string("loading of '" + filename + "' failed: no channels of audio found." ).c_str() );
	}

	for ( unsigned int i = 0; i < channels; ++i )
	{
        m_Data.push_back( std::vector< float >() );
        m_Data[i].resize((unsigned)sf_info.frames);
	}

    std::vector< float > buf;
    buf.resize( channels * (unsigned)sf_info.frames );

    if ( sf_info.frames != sf_readf_float( sf, &buf[0], sf_info.frames ) )
    {
        sf_close(sf);
        throw AudioException("could not read enough frames.");
    }

    for ( unsigned int i = 0; i < channels; ++i )
    {
        for ( unsigned frame = 0; frame < sf_info.frames; ++frame )
        {
            m_Data[ i ][frame] = buf[ (frame*channels) + i ];
        }
    }

	sf_close( sf );

	cout << "Finished reading '" + filename + "': " << channels << " channels, " << maxSize() << " samples at " << sample_rate_ << "hz." << endl;
}

unsigned AudioWave::sample_rate() const
{
	return sample_rate_;
}

float AudioWave::length() const
{
    return minSize() / (float) sample_rate();
}

bool AudioWave::saveToWav( const std::string& filename ) const
{
	assert( minSize() == maxSize() );

	if ( minSize() != maxSize() )
	{
		cout << "Cannot save to wav, minSize != maxSize" << endl;
		return false;
	}

	if ( sample_rate_ == 0 )
	{
		cout << "Cannot save to wav, sample rate is 0." << endl;
		return false;
	}

	const float sec = (float)minSize() / (float)sample_rate_;

	cout << "AudioWave::saveToWav: Going to write " << minSize() << " samples @ " << sample_rate_ << "hz (" << sec << " sec) to " << filename << "...";

	SF_INFO sf_info;

	memset( &sf_info, 0, sizeof( sf_info ) ) ;

	sf_info.samplerate = sample_rate_;
	sf_info.frames = minSize();
	sf_info.channels = channels();
	sf_info.format = (SF_FORMAT_WAV | SF_FORMAT_FLOAT);

	SNDFILE* sf = sf_open( filename.c_str(), SFM_WRITE, &sf_info );

	if ( !sf )
	{
		cout << "writing of '" << filename << "' failed: " << sf_strerror(sf) << endl;
		return false;
	}

    std::vector< float > buf;
    buf.resize( channels() * minSize() );

	for ( unsigned int i = 0; i < minSize(); ++i )
	{
		for ( unsigned c = 0; c < channels(); ++c )
		{
            buf[ (i * channels()) + c ] = getSamples( c )[ i ];
		}
	}

    if ( sf_writef_float( sf, &buf[0], minSize() ) != minSize() )
    {
        sf_close( sf );
        cout << "error writing bytes" << endl;
        return false;
    }

	sf_close( sf );

	cout << "done." << endl;

	return true;
}

void AudioWave::apply_hamming_window()
{
	std::vector< float > window( minSize(), 0.0f);
	hamming( &window[0], window.size());

	for ( unsigned channel = 0; channel < channels(); ++channel )
	{
		for ( unsigned i = 0; i < channelSize( channel ); ++i )
		{
			m_Data[ channel ][ i ] *= window[i];
		}
	}

}

void AudioWave::apply_hanning_window()
{
	std::vector< float > window( minSize(), 0.0f);
	hanning( &window[0], window.size());

	for ( unsigned channel = 0; channel < channels(); ++channel )
	{
		for ( unsigned i = 0; i < channelSize( channel ); ++i )
		{
			m_Data[ channel ][ i ] *= window[i];
		}
	}

}

void AudioWave::apply_blackman_window()
{
	std::vector< float > window( minSize(), 0.0f);
	blackman( &window[0], window.size());

	for ( unsigned channel = 0; channel < channels(); ++channel )
	{
		for ( unsigned i = 0; i < channelSize( channel ); ++i )
		{
			m_Data[ channel ][ i ] *= window[i];
		}
	}

}

void AudioWave::appendSample( unsigned int channel, float sample )
{
	if ( channel > channels() )
	{
		throw AudioException("appendSample: channel > channels()");
	}

	m_Data[ channel ].push_back( sample );
}

void AudioWave::appendSamples( unsigned int channel, const std::vector< float >& samples, unsigned offset, unsigned size )
{
	if ( channel > channels() )
	{
		throw AudioException("appendSamples: channel > channels()");
	}

    if ( offset > samples.size() || size > samples.size() || offset + size > samples.size() )
	{
		throw AudioException("appendSamples: offset + size > sample size");
	}

	m_Data[ channel ].insert( m_Data[ channel ].end(), samples.begin() + offset, samples.begin() + offset + size );
}

void AudioWave::appendSamples( unsigned int channel, const std::vector< float >& samples )
{
	appendSamples( channel, samples, 0, samples.size() );
}

void AudioWave::append( const AudioWave& other, unsigned offset, unsigned size )
{
	for ( unsigned int i = 0; i < other.channels() && i < channels(); ++i )
	{
		appendSamples( i, other.getSamples( i ), offset, size );
	}
}

void AudioWave::append( const AudioWave& other )
{
	for ( unsigned int i = 0; i < other.channels() && i < channels(); ++i )
	{
		appendSamples( i, other.getSamples( i ) );
	}
}

void AudioWave::add( const AudioWave& other, float gain )
{
    add( other, 0, 0, other.minSize(), gain );
}

void AudioWave::add( const AudioWave& other, unsigned dst_offset, unsigned src_offset, unsigned size, float gain )
{
	for ( unsigned int i = 0; i < other.channels() && i < channels(); ++i )
	{
        std::vector< float >& this_data = m_Data[i];
        const std::vector< float >& other_data = other.getSamples(i);

        const unsigned dst_start = std::min<unsigned>(dst_offset, this_data.size());
        const unsigned src_start = std::min<unsigned>(src_offset, other_data.size());

        assert( dst_start <= this_data.size());
        assert( src_start <= other_data.size());

        const unsigned dst_max = this_data.size() < dst_start ? 0 : (this_data.size() - dst_start);
        const unsigned src_max = other_data.size() < src_start ? 0 : (other_data.size() - src_start);

        const unsigned max_sample = std::min( dst_max, src_max );
        const unsigned real_size = std::min( max_sample, size );

        assert( src_start + real_size <= other_data.size() );
        assert( dst_start + real_size <= this_data.size() );

        for ( unsigned int s = 0; s < real_size; ++s )
		{
            this_data[dst_start + s] += gain * other_data[src_start + s];
		}
	}
}

const std::vector< float >& AudioWave::getSamples( unsigned int channel ) const
{
	if ( channel > channels() )
	{
		throw AudioException("getSamples: channel > channels()");
	}

	return m_Data[ channel ];
}

void AudioWave::fade_out( unsigned start, unsigned size )
{
	for ( unsigned int i = 0; i < m_Data.size(); ++i )
	{
		for ( unsigned int s = start; s < m_Data[ i ].size(); ++s )
		{
			float progress = (float)(s - start) / (float)size;

			progress = max( 0.0f, progress );
			progress = min( 1.0f, progress );

			float factor = 1.0f - progress;

			m_Data[i][s] *= factor;
		}
	}
}

std::vector< float >& AudioWave::getSamples( unsigned int channel )
{
	if ( channel > channels() )
	{
		throw AudioException("getSamples: channel > channels()");
	}

	return m_Data[ channel ];
}

unsigned int AudioWave::minSize() const
{
	unsigned int temp = 0;

	for ( unsigned int i = 0; i < m_Data.size(); ++i )
	{
		if ( i == 0 )
		{
			temp = m_Data[ i ].size();
		}

		temp = std::min< unsigned int >( temp, m_Data[ i ].size() );
	}

	return temp;
}

unsigned int AudioWave::maxSize() const
{
	unsigned int temp = 0;

	for ( unsigned int i = 0; i < m_Data.size(); ++i )
	{
		temp = std::max< unsigned int >( temp, m_Data[ i ].size() );
	}

	return temp;
}

void AudioWave::toStereo( float balance_left, float balance_right )
{
	if ( channels() != 1 )
	{
		return;
	}

	std::vector< float > left( maxSize(), 0.0f );
	std::vector< float > right( maxSize(), 0.0f );

	for ( unsigned int i = 0; i < maxSize(); ++i )
	{
		left[i] = m_Data[0][i] * balance_left;
		right[i] = m_Data[0][i] * balance_right;
	}

	m_Data.clear();
	m_Data.push_back( left );
	m_Data.push_back( right );
}

void AudioWave::mixToMono()
{
	// no data, no mono
	if ( m_Data.size() == 0 )
	{
		return;
	}

	std::vector< float > mono( maxSize(), 0.0f );

	// merge all into mono
	for ( unsigned int i = 0; i < m_Data.size(); ++i )
	{
		for ( unsigned int s = 0; s < m_Data[ i ].size(); ++s )
		{
			if ( s >= mono.size() ) // can't be actually.
			{
				assert( s < mono.size() );
				continue;
			}

			mono[ s ] += m_Data[ i ][ s ] / (float)m_Data.size();
		}
	}

	m_Data.clear();
	m_Data.push_back( mono );
}

unsigned int AudioWave::channelSize( unsigned int channel ) const
{
	if ( channel > channels() )
	{
		throw AudioException("channelSize: channel > channels()");
	}

	return m_Data[ channel ].size();
}

void AudioWave::scaleChannel( unsigned int channel, float factor )
{
	if ( channel > channels() )
	{
		throw AudioException("scaleChannel: channel > channels()");
	}

	for ( unsigned i = 0; i < channelSize( channel ); ++i )
	{
		m_Data[ channel ][ i ] *= factor;
	}
}

void AudioWave::scaleChannels( float factor )
{
	for ( unsigned int i = 0; i < channels(); ++i )
	{
		scaleChannel( i, factor );
	}
}

float AudioWave::maxLevel( unsigned int channel ) const
{
	if ( channel > channels() )
	{
		throw AudioException("maxLevel: channel > channels()");
	}

    std::vector< float >::const_iterator i_max = std::max_element( m_Data[ channel ].begin() , m_Data[ channel ].end() );
    std::vector< float >::const_iterator i_min = std::min_element( m_Data[ channel ].begin() , m_Data[ channel ].end() );

    float temp = 0.0f;

    if ( i_max != m_Data[ channel ].end() )
	{
        temp = std::max( temp, fabs(*i_max) );
	}

    if ( i_min != m_Data[ channel ].end() )
    {
        temp = std::max( temp, fabs(*i_min) );
    }

    return temp;
}

float AudioWave::maxLevel() const
{
	float temp = 0.0f;

	for ( unsigned int i = 0; i < channels(); ++i )
	{
		temp = std::max( temp, maxLevel( i ) );
	}

	return temp;
}

void AudioWave::normalize()
{
	if ( channels() == 0 || minSize() == 0 )
	{
		return; // nothing to normalize
	}

	const float fMaxLevel = maxLevel();

	if ( fMaxLevel == 0.0f )
	{
		return; // nothing to normalize
	}

	scaleChannels( 1.0f / fMaxLevel );
}

bool AudioWave::uniformSize() const
{
	return minSize() == maxSize();
}

unsigned int AudioWave::channels() const
{
	return m_Data.size();
}

void AudioWave::resize( unsigned newSize, float fillWith )
{
	for ( unsigned int i = 0; i < channels(); ++i )
	{
		m_Data[ i ].resize( newSize, fillWith );
	}
}

void AudioWave::append_stretched( const AudioWave & other, unsigned offset, unsigned size, double stretch_factor, bool high_quality )
{
	unsigned stretch_size = static_cast<unsigned>( (double)size * stretch_factor );
    unsigned before_size = minSize();

    resize(before_size+stretch_size);

	for ( unsigned int i = 0; i < other.channels() && i < channels(); ++i )
	{
        float* data_out = &m_Data[i][before_size];

        assert(m_Data[i].size() >= before_size + stretch_size);

		std::vector< float > in_samples = other.getSamples(i);

        if ( in_samples.size() <= offset )
        {
            continue;
        }

        unsigned real_in_size = std::min<unsigned>(in_samples.size() - offset, size);

		float* data_in = &in_samples[offset];

		SRC_DATA src_data;
		memset( &src_data, 0, sizeof( src_data ) );

		src_data.data_in = data_in;
		src_data.data_out = data_out;
        src_data.input_frames = real_in_size;
		src_data.output_frames = stretch_size;
		src_data.src_ratio = stretch_factor;

        src_simple( &src_data, high_quality ? SRC_SINC_BEST_QUALITY : SRC_SINC_FASTEST, 1 );
	}
}
