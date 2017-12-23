#ifndef __AREPA__AUDIO_PROCESSOR_H
#define __AREPA__AUDIO_PROCESSOR_H

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

	void start(const std::string& location, const std::string& device);
	void stop();
private:
	typedef std::vector<char> Buffer;

	struct CaptureChannel
	{
		boost::atomic<unsigned int> level;	// TODO: Maybe use float for percentage
		SndfileHandle * file;
	};
	typedef std::vector<CaptureChannel *> CaptureChannels;

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

			for (AudioProcessor::Records::iterator i = _records->begin(); i != _records->end(); ++i) {
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

	CaptureChannels _captureChannels;
	Records _records;

	const std::string _device;
	boost::atomic<snd_pcm_format_t> _format;
	boost::atomic<unsigned int> _rate;
	boost::atomic<unsigned int> _bytesPerSample;
	boost::atomic<unsigned int> _channels;
	boost::atomic<unsigned int> _periodSize;
	boost::atomic<std::size_t> _periodBufferSize;

	Buffer _captureRingBuffer;

	boost::condition_variable _captureCond;
	boost::mutex _captureMutex;
	std::size_t _captureOffset;
	std::size_t _ringsCaptured;
	std::size_t _recordOffset;
	std::size_t _ringsRecorded;
};

#endif
