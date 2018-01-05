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

class AudioProcessor
{
public:
	AudioProcessor(const char * device);
	~AudioProcessor();

	inline std::size_t getCaptureChannels() const
	{
		return _captureChannelsCount;
	}
	float getCaptureLevel(unsigned int channel, std::size_t after, std::size_t end);

	inline std::size_t getCapturedFrames() const
	{
		return _capturedFrames.load();
	}
	inline std::size_t getRecordStartedFrame() const
	{
		return _recordStartedFrame.load();
	}
	inline std::size_t getRecordFinishedFrame() const
	{
		return _recordFinishedFrame.load();
	}

	void startRecord(const char * location, const char * filenamePrefix);
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
	snd_pcm_format_t _format;
	std::size_t _rate;
	std::size_t _bytesPerSample;
	std::size_t _captureChannelsCount;
	std::size_t _framesInPeriod;
	std::size_t _periodBufferSize;

	boost::atomic<std::size_t> _capturedFrames;
	boost::atomic<std::size_t> _recordStartedFrame;
	boost::atomic<std::size_t> _recordFinishedFrame;

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
	std::string _filenamePrefix;
	std::size_t _periodsCaptured;
	std::size_t _periodsProcessed;
};

#endif
