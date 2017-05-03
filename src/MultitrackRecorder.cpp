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
	_access(),
	_format(),
	_subformat(),
	_rate(),
	_channels(),
	_periodTime(),
	_periodSize(),
	_bufferTime(),
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

	rc = snd_pcm_hw_params_set_channels(handle, params, 2);
	if (rc < 0) {
		std::cerr << "WARNING: Requesting channels amount error: " << snd_strerror(rc) <<std::endl;
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

	snd_pcm_hw_params_get_access(params, &_access);
	std::cout << "Access type: " << snd_pcm_access_name(_access) << std::endl;

	snd_pcm_hw_params_get_format(params, &_format);
	std::cout << "Format: " << snd_pcm_format_name(_format) << ", " << snd_pcm_format_description(_format) << std::endl;

	snd_pcm_hw_params_get_subformat(params, &_subformat);
	std::cout << "Subformat: " << snd_pcm_subformat_name(_subformat) << ", " << snd_pcm_subformat_description(_subformat) << std::endl;

	snd_pcm_hw_params_get_rate(params, &_rate, &dir);
	std::cout << "Sample rate: " << _rate << " bps" << std::endl;

	unsigned int channels;
	snd_pcm_hw_params_get_channels(params, &channels);
	_channels = channels;
	std::cout << "Channels: " << channels << std::endl;

	snd_pcm_hw_params_get_period_time(params, &_periodTime, &dir);
	std::cout << "Period time: " << _periodTime << " us" << std::endl;

	snd_pcm_uframes_t periodSize;
	snd_pcm_hw_params_get_period_size(params, &periodSize, &dir);
	_periodSize = periodSize;
	std::cout << "Period size: " << periodSize << " frames" << std::endl;

	snd_pcm_hw_params_get_buffer_time(params, &_bufferTime, &dir);
	std::cout << "Buffer time: " << _bufferTime << " us" << std::endl;

	// TODO: Correct sample bytes calculation
	std::size_t sampleBytes = 2;
	if (_format == SND_PCM_FORMAT_S32_LE) {
		sampleBytes = 4;
	} else if (_format != SND_PCM_FORMAT_S16_LE) {
		std::ostringstream msg;
		msg << "Unsupported format: " << snd_pcm_format_name(_format) << '(' <<
			snd_pcm_format_description(_format) << ')';
		throw std::runtime_error(msg.str());
	}
	_periodBufferSize = _periodSize * _channels * sampleBytes;
	std::cout << "Allocating " << _periodBufferSize << " bytes capture buffer" << std::endl;
	CaptureBuffer captureBuffer(_periodBufferSize);
	captureBuffer.resize(_periodBufferSize); // TODO: ???
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
	//std::ofstream wavFile();

	typedef std::vector<std::ofstream *> TargetFiles;
	TargetFiles targetFiles;

	class FilesCleanup {
	public:
		FilesCleanup(TargetFiles& targetFiles) :
			_targetFiles(targetFiles)
		{}

		~FilesCleanup()
		{
			// TODO Close files
			for (std::size_t i = 0U; i < _targetFiles.size(); ++i) {
				std::cout << "Closing " << i << "-th file" << std::endl;
				delete _targetFiles[i];
			}
			_targetFiles.clear();
		}

	private:
		TargetFiles& _targetFiles;
	} cleanup(targetFiles);

	CaptureBuffer recordBuffer;

	while (_shouldRun) {
		boost::unique_lock<boost::mutex> lock(_captureQueueMutex);
		if (_captureQueue.empty()) {
			continue;
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

		std::size_t data_chunk_pos = 0U;

		if (targetFiles.empty()) {
			recordBuffer.resize(_periodBufferSize);

			targetFiles.resize(_channels);
			for (std::size_t i = 0U; i < _channels; ++i) {
				std::ostringstream filename;
				filename << "track " << i << ".wav";
				std::cout << "Opening '" << filename.str() << "' file" << std::endl;
				std::ofstream * targetFile = new std::ofstream(filename.str().c_str(),
						std::ios_base::trunc | std::ios_base::binary);

				// Writing header
				*targetFile << "RIFF----WAVEfmt ";	// Chunk size to be filled in later
				write_word(*targetFile, 16, 4);		// No extension data
				write_word(*targetFile, 1, 2);		// TODO: PCM - integer samples
				write_word(*targetFile, 1, 2);		// One channel (mono file)
				write_word(*targetFile, 44100, 4);	// Samples per second (Hz)
				write_word(*targetFile, 88200, 4);	// TODO: (Sample Rate * BitsPerSample * Channels) / 8
				write_word(*targetFile, 2, 2);		// TODO: (NumChannels * BitsPerSample) / 8
				write_word(*targetFile, 16, 2);		// TODO: Number of bits per sample (use a multiple of 8)

				// Write the data chunk header
				if (data_chunk_pos == 0U) {
					data_chunk_pos = targetFile->tellp();
				}
				*targetFile << "data----";		// Chunk size to be filled in later

				targetFiles[i] = targetFile;
			}
		}

		bool debugPrinted = false;
		// Writing data to files
		while (!captureQueue.empty()) {
			CaptureBuffer captureBuffer = captureQueue.front();

			std::size_t bytesPerSample = 2U;	// TODO: Extract bytes per sample
			for (std::size_t i = 0; i < _periodBufferSize; i += bytesPerSample) {
				std::size_t frame = i / bytesPerSample / _channels;
				std::size_t channel = i / bytesPerSample % _channels;
				std::size_t j = channel * _periodSize * bytesPerSample + frame * bytesPerSample;

				/*if (!debugPrinted) {
					std::cout << ">>> captureBuffer.size(): " << captureBuffer.size() <<
						", recordBuffer.size(): " << recordBuffer.size() <<
						", _periodBufferSize: " << _periodBufferSize <<
						", _periodSize: " << _periodSize <<
						", i: " << i << ", frame: " << frame << ", channel: " << channel <<
						", j: " << j << std::endl;
				}*/

				memcpy(&recordBuffer[j], &captureBuffer[i], bytesPerSample);
			}
			debugPrinted = true;

			captureQueue.pop();

			for (std::size_t i = 0U; i < targetFiles.size(); ++i) {
				std::cout << ">>> Writing " << _periodSize * bytesPerSample << " bytes to " << i << "-th file" << std::endl;
				targetFiles[i]->write(&recordBuffer[i * _periodSize * bytesPerSample], _periodSize * bytesPerSample);
			}
		}

		// Updating headers
		for (std::size_t i = 0U; i < targetFiles.size(); ++i) {
			std::size_t file_length = targetFiles[i]->tellp();

			// Fix the data chunk header to contain the data size
			targetFiles[i]->seekp(data_chunk_pos + 4);
			write_word(*targetFiles[i], file_length - data_chunk_pos + 8, 4);

			// Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
			targetFiles[i]->seekp(4);
			write_word(*targetFiles[i], file_length - 8, 4);

			// Moving back to EOF
			//targetFiles[i]->seekp(file_length);
			targetFiles[i]->seekp(0, std::ios_base::end);
		}
	}
/*
	using namespace std;

	namespace little_endian_io
	{
		template <typename Word>
			std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
			{
				for (; size; --size, value >>= 8)
					outs.put( static_cast <char> (value & 0xFF) );
				return outs;
			}
	}
	using namespace little_endian_io;

	int main()
	{
		ofstream f( "example.wav", ios::binary );

		// Write the file headers
		f << "RIFF----WAVEfmt ";     // (chunk size to be filled in later)
		write_word( f,     16, 4 );  // no extension data
		write_word( f,      1, 2 );  // PCM - integer samples
		write_word( f,      2, 2 );  // two channels (stereo file)
		write_word( f,  44100, 4 );  // samples per second (Hz)
		write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
		write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
		write_word( f,     16, 2 );  // number of bits per sample (use a multiple of 8)

		// Write the data chunk header
		size_t data_chunk_pos = f.tellp();
		f << "data----";  // (chunk size to be filled in later)

		// Write the audio samples
		// (We'll generate a single C4 note with a sine wave, fading from left to right)
		constexpr double two_pi = 6.283185307179586476925286766559;
		constexpr double max_amplitude = 32760;  // "volume"

		double hz        = 44100;    // samples per second
		double frequency = 261.626;  // middle C
		double seconds   = 2.5;      // time

		int N = hz * seconds;  // total number of samples
		for (int n = 0; n < N; n++)
		{
			double amplitude = (double)n / N * max_amplitude;
			double value     = sin( (two_pi * n * frequency) / hz );
			write_word( f, (int)(                 amplitude  * value), 2 );
			write_word( f, (int)((max_amplitude - amplitude) * value), 2 );
		}

		// (We'll need the final file size to fix the chunk sizes above)
		size_t file_length = f.tellp();

		// Fix the data chunk header to contain the data size
		f.seekp( data_chunk_pos + 4 );
		write_word( f, file_length - data_chunk_pos + 8 );

		// Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
		f.seekp( 0 + 4 );
		write_word( f, file_length - 8, 4 ); 
	}
*/
}
