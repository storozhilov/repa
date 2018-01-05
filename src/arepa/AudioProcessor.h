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
	float getCaptureLevel(unsigned int channel, std::size_t after, std::size_t end);

	// TODO: Get rid of 3 methods below
	inline std::size_t getRecordStartedPeriod() const
	{
		return _recordStartedPeriod.load();
	}
	inline std::size_t getRecordFinishedPeriod() const
	{
		return _recordFinishedPeriod.load();
	}
	inline std::size_t getCapturedPeriods() const
	{
		return _capturedPeriods.load();
	}

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
	boost::atomic<unsigned int> _framesInPeriod;
	boost::atomic<std::size_t> _periodBufferSize;

	boost::atomic<std::size_t> _recordStartedPeriod; // TODO: Remove it
	boost::atomic<std::size_t> _recordFinishedPeriod; // TODO: Remove it
	boost::atomic<std::size_t> _capturedPeriods; // TODO: Remove it

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
	time_t _recordTs;
	std::size_t _periodsCaptured;
	std::size_t _periodsProcessed;
};

#endif
