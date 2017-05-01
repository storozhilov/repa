#ifndef __SR__MULTITRACK_RECORDER_H
#define __SR__MULTITRACK_RECORDER_H

#include <vector>

#include <boost/atomic.hpp>
#include <boost/thread.hpp>

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const std::string& location, const std::string& device);
	void stop();
private:
	void runCapture();
	void runRecord();

	boost::atomic<bool> _shouldRun;
	boost::thread _captureThread;
	boost::thread _recordThread;

	snd_pcm_t * _captureHandle;
	snd_pcm_access_t _access;
	snd_pcm_format_t _format;
	snd_pcm_subformat_t _subformat;
	unsigned int _rate;
	unsigned int _channels;
	unsigned int _periodTime;
	snd_pcm_uframes_t _periodSize;
	unsigned int _bufferTime;

	std::vector<char> _captureBuffer;
};

#endif
