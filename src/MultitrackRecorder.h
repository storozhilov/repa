#ifndef __REPA__MULTITRACK_RECORDER_H
#define __REPA__MULTITRACK_RECORDER_H

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

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const std::string& location, const std::string& device);
	void stop();
private:
	typedef std::vector<char> Buffer;
	typedef std::queue<Buffer> CaptureQueue;
	typedef std::map<std::size_t, SndfileHandle *> Records;

	class RecordsCleaner {
	public:
		RecordsCleaner(Records& records) :
			_records(&records)
		{}

		~RecordsCleaner()
		{
			if (_records == 0) {
				return;
			}

			for (MultitrackRecorder::Records::iterator i = _records->begin(); i != _records->end(); ++i) {
				delete (*i).second;
			}
			_records->clear();
		}

		void release() {
			_records = 0;
		}
	private:
		Records * _records;
	};

	void runCapture();
	void runRecord();

	boost::atomic<bool> _shouldRun;
	boost::thread _captureThread;
	boost::thread _recordThread;

	snd_pcm_t * _handle;

	Records _records;

	boost::atomic<snd_pcm_format_t> _format;
	boost::atomic<unsigned int> _rate;
	boost::atomic<unsigned int> _bytesPerSample;
	boost::atomic<unsigned int> _channels;
	boost::atomic<unsigned int> _periodSize;
	boost::atomic<std::size_t> _periodBufferSize;

	Buffer _captureRingBuffer;
	Buffer _recordBuffer;

	CaptureQueue _captureQueue;
	boost::condition_variable _captureQueueCond;
	boost::mutex _captureQueueMutex;
	std::size_t _captureOffset;
	std::size_t _ringsCaptured;
	std::size_t _recordOffset;
	std::size_t _ringsRecorded;
};

#endif
