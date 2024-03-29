#include "asoundlib.h"

#include "pcm.h"
#include "player.h"

#include <array>
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <iostream>

Player::Player(const Wave &wave) :
    _samplerate(wave.samplerate()),
    _data(wave.wave())
{
}

void Player::play()
{

    char const *device = "default";			/* playback device */
    int err;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_pcm_set_params(handle,
	                              SND_PCM_FORMAT_FLOAT_LE,
	                              SND_PCM_ACCESS_RW_INTERLEAVED,
	                              1,
	                              11025,
	                              1,
	                              500000)) < 0) {	/* 0.5sec */
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

    for (auto dt_i = _data.cbegin(); dt_i != _data.cend(); )
    {
        size_t left_data = std::distance(dt_i, _data.end());
        size_t copied_frames = (left_data > _buffer.size()) ? _buffer.size() : left_data;
        std::copy_n(dt_i, copied_frames, _buffer.begin());
        frames = snd_pcm_writei(handle, _buffer.data(), _buffer.size());

        if (frames < 0)
        {
            frames = snd_pcm_recover(handle, frames, 0);
            std::cout << "snd_pcm_writei failed: " << snd_strerror(frames) << '\n';
        }

        if (frames > 0 && frames < (long) _buffer.size())
        {
            std::cout << "Short write (epected " << _buffer.size() << ", wrote " << frames << ")\n";
        }

        std::advance(dt_i, copied_frames);
        _buffer.fill(0.);
    }

    snd_pcm_close(handle);
}
