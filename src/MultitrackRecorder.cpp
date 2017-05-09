#include "MultitrackRecorder.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstring>
#include <boost/filesystem.hpp>

namespace
{

class AlsaCleaner {
public:
	AlsaCleaner(snd_pcm_t * handle) :
		_handle(handle)
	{}

	~AlsaCleaner()
	{
		if (_handle == 0) {
			return;
		}

		int rc = snd_pcm_drain(_handle);
		if (rc < 0) {
			std::ostringstream msg;
			msg << "Stopping PCM device error: " << snd_strerror(rc);
			throw std::runtime_error(msg.str());
		}
		rc = snd_pcm_close(_handle);
		if (rc < 0) {
			std::ostringstream msg;
			msg << "Closing PCM device error: " << snd_strerror(rc);
			throw std::runtime_error(msg.str());
		}
	}

	void release() {
		_handle = 0;
	}
private:
	snd_pcm_t * _handle;
};

}


MultitrackRecorder::MultitrackRecorder() :
	_shouldRun(false),
	_captureThread(),
	_recordThread(),
	_handle(),
	_records(),
	_format(),
	_rate(),
	_bytesPerSample(),
	_channels(),
	_periodSize(),
	_periodBufferSize(),
	_captureQueue(),
	_captureQueueCond(),
	_captureQueueMutex()
{
	std::cout << "ALSA library version: " << SND_LIB_VERSION_STR << std::endl;
	std::cout << "PCM stream types:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_STREAM_LAST; i++) {
		std::cout << " - " << snd_pcm_stream_name(static_cast<snd_pcm_stream_t>(i)) << std::endl;
	}
	std::cout << "PCM access types:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_ACCESS_LAST; i++) {
		std::cout << " - " << snd_pcm_access_name(static_cast<snd_pcm_access_t>(i)) << std::endl;
	}
	std::cout << "PCM formats:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_FORMAT_LAST; i++) {
		if (snd_pcm_format_name(static_cast<snd_pcm_format_t>(i))) {
			std::cout << " - " << snd_pcm_format_name(static_cast<snd_pcm_format_t>(i)) <<
				": " << snd_pcm_format_description(static_cast<snd_pcm_format_t>(i)) << std::endl;
		}
	}
	std::cout << "PCM subformats:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_SUBFORMAT_LAST; i++) {
		std::cout << " - " << snd_pcm_subformat_name(static_cast<snd_pcm_subformat_t>(i)) <<
			": " << snd_pcm_subformat_description(static_cast<snd_pcm_subformat_t>(i)) << std::endl;
	}
	std::cout << "PCM states:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_STATE_LAST; i++) {
		std::cout << " - " << snd_pcm_state_name(static_cast<snd_pcm_state_t>(i)) << std::endl;
	}
}

void MultitrackRecorder::start(const std::string& location, const std::string& device)
{
	// ALSA intialization
	int rc = snd_pcm_open(&_handle, device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Opening PCM device error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}
	AlsaCleaner alsaCleaner(_handle);

	snd_pcm_hw_params_t * params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(_handle, params);

	rc = snd_pcm_hw_params_set_access(_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting access type error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params_set_format(_handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0) {
		std::cerr << "WARNING: Requesting format error: " << snd_strerror(rc) <<std::endl;
	}

	unsigned int val;
	int dir;

	val = 44100;
	rc = snd_pcm_hw_params_set_rate_near(_handle, params, &val, &dir);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting sample rate error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params(_handle, params);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Setting H/W parameters error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	std::cout << "PCM handle name: " << snd_pcm_name(_handle) << std::endl <<
		"PCM state: " << snd_pcm_state_name(snd_pcm_state(_handle)) << std::endl;

	snd_pcm_access_t access;
	snd_pcm_hw_params_get_access(params, &access);
	std::cout << "Access type: " << snd_pcm_access_name(access) << std::endl;

	snd_pcm_format_t format;
	snd_pcm_hw_params_get_format(params, &format);
	_format.store(format);
	std::cout << "Format: " << snd_pcm_format_name(format) << ", " << snd_pcm_format_description(format) << std::endl;

	snd_pcm_subformat_t subformat;
	snd_pcm_hw_params_get_subformat(params, &subformat);
	std::cout << "Subformat: " << snd_pcm_subformat_name(subformat) << ", " << snd_pcm_subformat_description(subformat) << std::endl;

	unsigned int rate;
	snd_pcm_hw_params_get_rate(params, &rate, &dir);
	_rate.store(rate);
	std::cout << "Sample rate: " << rate << " bps" << std::endl;

	unsigned int channels;
	snd_pcm_hw_params_get_channels(params, &channels);
	_channels.store(channels);
	std::cout << "Channels: " << channels << std::endl;

	unsigned int periodTime;
	snd_pcm_hw_params_get_period_time(params, &periodTime, &dir);
	std::cout << "Period time: " << periodTime << " us" << std::endl;

	snd_pcm_uframes_t periodSize;
	snd_pcm_hw_params_get_period_size(params, &periodSize, &dir);
	_periodSize.store(periodSize);
	std::cout << "Period size: " << periodSize << " frames" << std::endl;

	unsigned int bufferTime;
	snd_pcm_hw_params_get_buffer_time(params, &bufferTime, &dir);
	std::cout << "Buffer time: " << bufferTime << " us" << std::endl;

	// TODO: Correct bytes per sample calculation
	std::size_t bytesPerSample = 2;
	if (format == SND_PCM_FORMAT_S32_LE) {
		bytesPerSample = 4;
	} else if (format != SND_PCM_FORMAT_S16_LE) {
		std::ostringstream msg;
		msg << "Unsupported format: " << snd_pcm_format_name(format) << '(' <<
			snd_pcm_format_description(format) << ')';
		throw std::runtime_error(msg.str());
	}
	_bytesPerSample.store(bytesPerSample);
	std::cout << "Bytes per sample: " << bytesPerSample << std::endl;

	std::size_t periodBufferSize = periodSize * channels * bytesPerSample;
	_periodBufferSize.store(periodBufferSize);

	// Records initialization
	RecordsCleaner recordsCleaner(_records);
	for (std::size_t i = 1; i <= channels; ++i) {
		std::ostringstream filename;
		filename << "track_" << std::setfill('0') << std::setw(2) << i << ".wav";
		boost::filesystem::path fullPath = boost::filesystem::path(location) /
			boost::filesystem::path(filename.str());
		std::cout << "Opening '" << fullPath.native() << "' file" << std::endl;

		int fileFormat = SF_FORMAT_WAV;

		switch (format) {
			case SND_PCM_FORMAT_S16_LE:
				fileFormat |= SF_FORMAT_PCM_16;
				break;
			case SND_PCM_FORMAT_S32_LE:
				fileFormat |= SF_FORMAT_PCM_32;
				break;
			case SND_PCM_FORMAT_FLOAT_LE:
				fileFormat |= SF_FORMAT_FLOAT;
				break;
			default:
				std::ostringstream msg;
				msg << "WAV-file format is not supported: " << snd_pcm_format_name(format) <<
					", " << snd_pcm_format_description(format);
				throw std::runtime_error(msg.str());
		}
		_records.insert(Records::value_type(i, new SndfileHandle(fullPath.c_str(),
						SFM_WRITE, fileFormat, 1, rate)));
	}

	// Everything is fine - releasing cleaners
	alsaCleaner.release();
	recordsCleaner.release();

	// Starting capture & record threads
	_shouldRun = true;
	_captureThread = boost::thread(boost::bind(&MultitrackRecorder::runCapture, this));
	_recordThread = boost::thread(boost::bind(&MultitrackRecorder::runRecord, this, location));
}

void MultitrackRecorder::stop()
{
	_shouldRun = false;

	boost::unique_lock<boost::mutex> lock(_captureQueueMutex);
	_captureQueueCond.notify_all();
	lock.unlock();

	_captureThread.join();
	_recordThread.join();
}

void MultitrackRecorder::runCapture()
{
	AlsaCleaner alsaCleaner(_handle);
	std::size_t periodSize = _periodSize.load();
	std::size_t periodBufferSize = _periodBufferSize.load();

	CaptureBuffer captureBuffer(periodBufferSize);

	while (_shouldRun) {
		int rc = snd_pcm_readi(_handle, &captureBuffer[0], periodSize);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			std::cerr << "ERROR: Overrun occurred" << std::endl;
			snd_pcm_prepare(_handle);
			continue;
		} else if (rc < 0) {
			std::cerr << "ERROR: Reading data error: " << snd_strerror(rc) << std::endl;
			return;
		} else if (rc != periodSize) {
			// TODO: Special handling
			std::cerr << "WARNING: Short read: " << rc << "/" << periodSize << " frames" << std::endl;
		}
		std::cout << '.';
		boost::unique_lock<boost::mutex> lock(_captureQueueMutex);
		_captureQueue.push(captureBuffer);
		_captureQueueCond.notify_all();
	}
}

void MultitrackRecorder::runRecord(const std::string& location)
{
	RecordsCleaner recordsCleaner(_records);

	snd_pcm_format_t format = _format.load();
	unsigned int bytesPerSample = _bytesPerSample.load();
	unsigned int channels = _channels.load();
	unsigned int periodSize = _periodSize.load();
	unsigned int periodBufferSize = _periodBufferSize.load();

	CaptureBuffer recordBuffer(periodBufferSize);

	while (_shouldRun) {
		boost::unique_lock<boost::mutex> lock(_captureQueueMutex);
		while (_captureQueue.empty()) {
			_captureQueueCond.wait(lock);
			if (!_shouldRun) {
				break;
			}
		}
		//CaptureQueue captureQueue;
		//captureQueue.swap(_captureQueue);
		CaptureQueue captureQueue(_captureQueue);
		while (!_captureQueue.empty()) {
			_captureQueue.pop();
		}
		lock.unlock();

		for (std::size_t i = 0U; i < captureQueue.size(); ++i) {
			std::cout << '+';
		}

		while (!captureQueue.empty()) {
			CaptureBuffer captureBuffer = captureQueue.front();

			for (std::size_t i = 0; i < periodBufferSize; i += bytesPerSample) {
				std::size_t frame = i / bytesPerSample / channels;
				std::size_t channel = i / bytesPerSample % channels;
				std::size_t j = channel * _periodSize * bytesPerSample + frame * bytesPerSample;

				memcpy(&recordBuffer[j], &captureBuffer[i], bytesPerSample);
			}

			captureQueue.pop();

			for (std::size_t i = 0U; i < channels; ++i) {
				sf_count_t itemsToWrite = 0;
				sf_count_t itemsWritten = 0;

				char * buf = &recordBuffer[i * _periodSize * bytesPerSample];
				std::size_t size = _periodSize * bytesPerSample;

				switch (format) {
					case SND_PCM_FORMAT_S16_LE:
						itemsToWrite = size / 2;
						itemsWritten = _records[i + 1]->write(reinterpret_cast<short *>(buf), itemsToWrite);
						break;
					case SND_PCM_FORMAT_S32_LE:
						itemsToWrite = size / 4;
						itemsWritten = _records[i + 1]->write(reinterpret_cast<int *>(buf), itemsToWrite);
						break;
					case SND_PCM_FORMAT_FLOAT_LE:
						itemsToWrite = size / 4;
						itemsWritten = _records[i + 1]->write(reinterpret_cast<float *>(buf), itemsToWrite);
						break;
					default:
						std::ostringstream msg;
						msg << "WAV-file format is not supported: " << snd_pcm_format_name(format) <<
							", " << snd_pcm_format_description(format);
						throw std::runtime_error(msg.str());
				}

				if (itemsWritten != itemsToWrite) {
					std::ostringstream msg;
					msg << "Unconsistent written items count: " << itemsWritten << '/' << itemsToWrite;
					throw std::runtime_error(msg.str());
				}
			}
		}
	}
}
