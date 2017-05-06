#include "MultitrackRecorder.h"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>

MultitrackRecorder::MultitrackRecorder() :
	_shouldRun(false),
	_captureThread(),
	_recordThread(),
	_rate(),
	_bytesPerSample(),
	_channels(),
	_periodSize(),
	_periodBufferSize(0U),
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
	_shouldRun = true;
	_captureThread = boost::thread(boost::bind(&MultitrackRecorder::runCapture, this, device));
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

void MultitrackRecorder::runCapture(const std::string& device)
{
	snd_pcm_t * handle;
	int rc = snd_pcm_open(&handle, device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Opening PCM device error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	class AlsaCleanup {
	public:
		AlsaCleanup(snd_pcm_t * handle) :
			_handle(handle)
		{}

		~AlsaCleanup()
		{
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
	private:
		snd_pcm_t * _handle;
	} alsaCleanup(handle);

	snd_pcm_hw_params_t * params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);

	rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting access type error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0) {
		std::cerr << "WARNING: Requesting format error: " << snd_strerror(rc) <<std::endl;
	}

	unsigned int val;
	int dir;

	val = 44100;
	rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting sample rate error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Setting H/W parameters error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	std::cout << "PCM handle name: " << snd_pcm_name(handle) << std::endl <<
		"PCM state: " << snd_pcm_state_name(snd_pcm_state(handle)) << std::endl;

	snd_pcm_access_t access;
	snd_pcm_hw_params_get_access(params, &access);
	std::cout << "Access type: " << snd_pcm_access_name(access) << std::endl;

	snd_pcm_format_t format;
	snd_pcm_hw_params_get_format(params, &format);
	std::cout << "Format: " << snd_pcm_format_name(format) << ", " << snd_pcm_format_description(format) << std::endl;

	snd_pcm_subformat_t subformat;
	snd_pcm_hw_params_get_subformat(params, &subformat);
	std::cout << "Subformat: " << snd_pcm_subformat_name(subformat) << ", " << snd_pcm_subformat_description(subformat) << std::endl;

	unsigned int rate;
	snd_pcm_hw_params_get_rate(params, &rate, &dir);
	_rate = rate;
	std::cout << "Sample rate: " << _rate << " bps" << std::endl;

	unsigned int channels;
	snd_pcm_hw_params_get_channels(params, &channels);
	_channels = channels;
	std::cout << "Channels: " << channels << std::endl;

	unsigned int periodTime;
	snd_pcm_hw_params_get_period_time(params, &periodTime, &dir);
	std::cout << "Period time: " << periodTime << " us" << std::endl;

	snd_pcm_uframes_t periodSize;
	snd_pcm_hw_params_get_period_size(params, &periodSize, &dir);
	_periodSize = periodSize;
	std::cout << "Period size: " << periodSize << " frames" << std::endl;

	unsigned int bufferTime;
	snd_pcm_hw_params_get_buffer_time(params, &bufferTime, &dir);
	std::cout << "Buffer time: " << bufferTime << " us" << std::endl;

	// TODO: Correct sample bytes calculation
	std::size_t bytesPerSample = 2;
	if (format == SND_PCM_FORMAT_S32_LE) {
		bytesPerSample = 4;
	} else if (format != SND_PCM_FORMAT_S16_LE) {
		std::ostringstream msg;
		msg << "Unsupported format: " << snd_pcm_format_name(format) << '(' <<
			snd_pcm_format_description(format) << ')';
		throw std::runtime_error(msg.str());
	}
	_bytesPerSample = bytesPerSample;
	std::cout << "Bytes per sample: " << _bytesPerSample << std::endl;
	_periodBufferSize = _periodSize * _channels * bytesPerSample;
	std::cout << "Allocating " << _periodBufferSize << " bytes capture buffer" << std::endl;
	CaptureBuffer captureBuffer(_periodBufferSize);
	// TODO: Other params

	while (_shouldRun) {
		int rc = snd_pcm_readi(handle, &captureBuffer[0], _periodSize);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			std::cerr << "ERROR: Overrun occurred" << std::endl;
			snd_pcm_prepare(handle);
			continue;
		} else if (rc < 0) {
			std::cerr << "ERROR: Reading data error: " << snd_strerror(rc) << std::endl;
			return;
		} else if (rc != _periodSize) {
			// TODO: Special handling
			std::cerr << "WARNING: Short read, read " << rc << "/" << _periodSize << " frames" << std::endl;
		}
		std::cout << '.';
		boost::unique_lock<boost::mutex> lock(_captureQueueMutex);
		_captureQueue.push(captureBuffer);
		_captureQueueCond.notify_all();
	}
}

namespace
{

template <typename Word> std::ostream& write_word(std::ostream& outs, Word value, std::size_t size = sizeof(Word))
{
	for (; size; --size, value >>= 8) {
		outs.put(static_cast<char>(value & 0xFF));
	}
	return outs;
}

}

void MultitrackRecorder::runRecord(const std::string& location)
{
	typedef std::vector<std::ofstream *> TargetFiles;
	TargetFiles targetFiles;

	class FilesCleanup {
	public:
		FilesCleanup(TargetFiles& targetFiles) :
			_targetFiles(targetFiles)
		{}

		~FilesCleanup()
		{
			// Closing files
			for (std::size_t i = 0U; i < _targetFiles.size(); ++i) {
				delete _targetFiles[i];
			}
			_targetFiles.clear();
		}

	private:
		TargetFiles& _targetFiles;
	} cleanup(targetFiles);

	CaptureBuffer recordBuffer;

	std::size_t dataChunkPos = 0U;
	bool streamDataInitialized = false;
	unsigned int rate;
	unsigned int bytesPerSample;
	unsigned int channels;

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

		if (!streamDataInitialized) {
			rate = _rate;
			bytesPerSample = _bytesPerSample;
			channels = _channels;
			streamDataInitialized = true;
		}

		if (targetFiles.empty()) {
			recordBuffer.resize(_periodBufferSize);

			targetFiles.resize(channels);
			for (std::size_t i = 0U; i < channels; ++i) {
				std::ostringstream filename;
				filename << "track_" << i << ".wav";
				std::cout << "Opening '" << filename.str() << "' file" << std::endl;
				std::ofstream * targetFile = new std::ofstream(filename.str().c_str(),
						std::ios_base::trunc | std::ios_base::binary);

				// Writing header
				*targetFile << "RIFF----WAVEfmt ";			// Chunk size to be filled in later
				write_word(*targetFile, 16, 4);				// No extension data
				write_word(*targetFile, 1, 2);				// 1 - PCM audio format
				write_word(*targetFile, 1, 2);				// One channel (mono file)
				write_word(*targetFile, rate, 4);			// Samples per second (Hz)
				write_word(*targetFile, rate * bytesPerSample, 4);	// (Sample Rate * BitsPerSample * Channels) / 8
				write_word(*targetFile, bytesPerSample, 2);		// (NumChannels * BitsPerSample) / 8
				write_word(*targetFile, bytesPerSample * 8, 2);		// Number of bits per sample (use a multiple of 8)

				// Write the data chunk header
				if (dataChunkPos == 0U) {
					dataChunkPos = targetFile->tellp();
				}
				*targetFile << "data----";		// Chunk size to be filled in later

				targetFiles[i] = targetFile;
			}
		}

		// Writing data to files
		while (!captureQueue.empty()) {
			CaptureBuffer captureBuffer = captureQueue.front();

			for (std::size_t i = 0; i < _periodBufferSize; i += bytesPerSample) {
				std::size_t frame = i / bytesPerSample / channels;
				std::size_t channel = i / bytesPerSample % channels;
				std::size_t j = channel * _periodSize * bytesPerSample + frame * bytesPerSample;

				memcpy(&recordBuffer[j], &captureBuffer[i], bytesPerSample);
			}

			captureQueue.pop();

			for (std::size_t i = 0U; i < targetFiles.size(); ++i) {
				targetFiles[i]->write(&recordBuffer[i * _periodSize * bytesPerSample], _periodSize * bytesPerSample);
			}
		}

		// Updating headers
		for (std::size_t i = 0U; i < targetFiles.size(); ++i) {
			std::size_t fileLength = targetFiles[i]->tellp();

			std::size_t chunkSize = fileLength - 8;
			std::size_t subchunkSize = fileLength - dataChunkPos - 8;

			// Fix the data chunk header to contain the data size
			targetFiles[i]->seekp(dataChunkPos + 4);
			write_word(*targetFiles[i], subchunkSize, 4);

			// Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
			targetFiles[i]->seekp(4);
			write_word(*targetFiles[i], chunkSize, 4);

			// Moving back to EOF
			targetFiles[i]->seekp(0, std::ios_base::end);
		}
	}
}
