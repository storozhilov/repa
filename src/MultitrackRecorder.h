#ifndef __SR__MULTITRACK_RECORDER_H
#define __SR__MULTITRACK_RECORDER_H

#include <vector>
#include <queue>

#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const std::string& location, const std::string& device);
	void stop();
private:
	typedef std::vector<char> PeriodBuffer;
	typedef std::queue<PeriodBuffer> PeriodsQueue;

	void runCapture(const std::string& device);
	void runRecord(const std::string& location);

	boost::atomic<bool> _shouldRun;
	boost::thread _captureThread;
	boost::thread _recordThread;

	// TODO: Make atomic or remove
	snd_pcm_access_t _access;
	snd_pcm_format_t _format;
	snd_pcm_subformat_t _subformat;
	unsigned int _rate;
	boost::atomic<unsigned int> _channels;
	unsigned int _periodTime;
	snd_pcm_uframes_t _periodSize;
	unsigned int _bufferTime;

	boost::atomic<std::size_t> _periodBufferSize;
	PeriodsQueue _periodsQueue;
	boost::condition_variable _periodsQueueCond;
	boost::mutex _periodsQueueMutex;
};

#endif
