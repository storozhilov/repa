#ifndef __REPA__MULTITRACK_RECORDER_H
#define __REPA__MULTITRACK_RECORDER_H

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <vector>
#include <queue>

#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const std::string& location, const std::string& device);
	void stop();
private:
	typedef std::vector<char> CaptureBuffer;
	typedef std::queue<CaptureBuffer> CaptureQueue;

	void runCapture();
	void runRecord(const std::string& location);

	boost::atomic<bool> _shouldRun;
	boost::thread _captureThread;
	boost::thread _recordThread;

	snd_pcm_t * _handle;
	CaptureBuffer _captureBuffer;

	boost::atomic<unsigned int> _format;
	boost::atomic<unsigned int> _rate;
	boost::atomic<unsigned int> _bytesPerSample;
	boost::atomic<unsigned int> _channels;
	boost::atomic<unsigned int> _periodSize;
	boost::atomic<std::size_t> _periodBufferSize;

	CaptureQueue _captureQueue;
	boost::condition_variable _captureQueueCond;
	boost::mutex _captureQueueMutex;
};

#endif
