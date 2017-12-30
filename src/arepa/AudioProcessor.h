#ifndef __AREPA__AUDIO_PROCESSOR_H
#define __AREPA__AUDIO_PROCESSOR_H

#include "CaptureChannel.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <sndfile.hh>

#include <vector>
#include <queue>
#include <map>

#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <ctime>

class AudioProcessor
{
public:
	AudioProcessor(const char * device);
	~AudioProcessor();

	inline unsigned int getCaptureChannels() const
	{
		return _captureChannelsCount.load();
	}
	float getCaptureLevel(unsigned int channel);

	time_t startRecord(const std::string& location);
	void stopRecord();
private:
	typedef std::vector<char> Buffer;
	typedef std::vector<CaptureChannel *> CaptureChannels;

	void runCapture();
	void runCapturePostProcessing();

	boost::thread _captureThread;
	boost::thread _capturePostProcessingThread;

	snd_pcm_t * _handle;

	CaptureChannels _captureChannels;

	const std::string _device;
	boost::atomic<snd_pcm_format_t> _format;
	boost::atomic<unsigned int> _rate;
	boost::atomic<unsigned int> _bytesPerSample;
	boost::atomic<unsigned int> _captureChannelsCount;
	boost::atomic<unsigned int> _periodSize;
	boost::atomic<std::size_t> _periodBufferSize;

	Buffer _captureRingBuffer;

	enum State {
		IdleState,
		CaptureStartingState,
		CaptureState,
		RecordRequestedState,
		RecordRequestConfirmedState,
		RecordState,
		CaptureRequestedState,
		CaptureRequestConfirmedState
	};

	boost::condition_variable _captureCond;
	boost::mutex _captureMutex;
	State _state;
	std::string _filesLocation;
	time_t _recordTs;
	std::size_t _captureOffset;
	std::size_t _ringsCaptured;
	std::size_t _recordOffset;
	std::size_t _ringsRecorded;
};

#endif
