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

	void start(const std::string& location, const std::string& device) {}
	// TODO: Use timestamp for records marking
	void startRecord(const std::string& location, std::size_t recordNumber = 0);
	void stopRecord();
	void stop() {}
private:
	typedef std::vector<char> Buffer;

	// TODO: Make a separate class & implement autotests
	class CaptureChannel
	{
	public:
		CaptureChannel(unsigned int rate, snd_pcm_format_t alsaFormat);
		~CaptureChannel();

		void openFile(const std::string& filename);
		void closeFile();
		inline bool fileIsOpen() const
		{
			return _file;
		}
		void write(const char * buf, std::size_t size);
	private:
		unsigned int _rate;
		snd_pcm_format_t _alsaFormat;
		int _sfFormat;
		boost::atomic<unsigned int> _level;	// TODO: Maybe use float for percentage
		std::string _filename;
		SndfileHandle * _file;
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
	void runCapturePostProcessing();

	boost::thread _captureThread;
	boost::thread _capturePostProcessingThread;

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
	std::size_t _recordNumber;
	std::size_t _captureOffset;
	std::size_t _ringsCaptured;
	std::size_t _recordOffset;
	std::size_t _ringsRecorded;
};

#endif
