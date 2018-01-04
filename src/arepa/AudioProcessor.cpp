#include "AudioProcessor.h" 
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <cstring>
#include <cassert>
#include <boost/filesystem.hpp>
#include <cassert>

namespace
{

// TODO: Make it configurable
std::size_t const PeriodsInBuffer = 1024U;

}

AudioProcessor::AudioProcessor(const char * device) :
	_device(device),
	_captureThread(),
	_capturePostProcessingThread(),
	_handle(),
	_captureChannels(),
	_format(),
	_rate(),
	_bytesPerSample(),
	_captureChannelsCount(),
	_framesInPeriod(),
	_periodBufferSize(),
	_recordStartedPeriod(),
	_recordFinishedPeriod(),
	_capturedPeriods(0U),
	_captureRingBuffer(),
	_captureCond(),
	_captureMutex(),
	_state(IdleState),
	_filesLocation(),
	_recordTs(0U),
	_periodsCaptured(0U),
	_periodsProcessed(0U)
{
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): ALSA library version: " <<
		SND_LIB_VERSION_STR << std::endl;
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM stream types:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_STREAM_LAST; i++) {
		std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'):  - " <<
			snd_pcm_stream_name(static_cast<snd_pcm_stream_t>(i)) << std::endl;
	}
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM access types:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_ACCESS_LAST; i++) {
		std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'):  - " <<
			snd_pcm_access_name(static_cast<snd_pcm_access_t>(i)) << std::endl;
	}
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM formats:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_FORMAT_LAST; i++) {
		if (snd_pcm_format_name(static_cast<snd_pcm_format_t>(i))) {
			std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'):  - " <<
				snd_pcm_format_name(static_cast<snd_pcm_format_t>(i)) <<
				": " << snd_pcm_format_description(static_cast<snd_pcm_format_t>(i)) << std::endl;
		}
	}
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM subformats:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_SUBFORMAT_LAST; i++) {
		std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'):  - " <<
			snd_pcm_subformat_name(static_cast<snd_pcm_subformat_t>(i)) <<
			": " << snd_pcm_subformat_description(static_cast<snd_pcm_subformat_t>(i)) << std::endl;
	}
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM states:" << std::endl;
	for (size_t i = 0; i <= SND_PCM_STATE_LAST; i++) {
		std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'):  - " <<
			snd_pcm_state_name(static_cast<snd_pcm_state_t>(i)) << std::endl;
	}

	// ALSA intialization
	int rc = snd_pcm_open(&_handle, _device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Opening '" << _device << "' PCM device error: " << snd_strerror(rc);
		std::cerr << "ERROR: AudioProcessor::AudioProcessor('" << device << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}

	snd_pcm_hw_params_t * params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(_handle, params);

	rc = snd_pcm_hw_params_set_access(_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting access type error: " << snd_strerror(rc);
		std::cerr << "ERROR: AudioProcessor::AudioProcessor('" << device << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params_set_format(_handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0) {
		std::cerr << "WARNING: AudioProcessor::AudioProcessor('" << device <<
			"'): Requesting format error: " << snd_strerror(rc) <<std::endl;
	}

	unsigned int val = 44100;
	int dir;
	rc = snd_pcm_hw_params_set_rate_near(_handle, params, &val, &dir);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting sample rate error: " << snd_strerror(rc);
		std::cerr << "ERROR: AudioProcessor::AudioProcessor('" << device << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params(_handle, params);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Setting H/W parameters error: " << snd_strerror(rc);
		std::cerr << "ERROR: AudioProcessor::AudioProcessor('" << device << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}

	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM handle name: " <<
		snd_pcm_name(_handle) << std::endl <<
		"NOTICE: AudioProcessor::AudioProcessor('" << device << "'): PCM state: " <<
		snd_pcm_state_name(snd_pcm_state(_handle)) << std::endl;

	snd_pcm_access_t access;
	snd_pcm_hw_params_get_access(params, &access);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Access type: " <<
		snd_pcm_access_name(access) << std::endl;

	snd_pcm_format_t format;
	snd_pcm_hw_params_get_format(params, &format);
	_format.store(format);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Format: " <<
		snd_pcm_format_name(format) << ", " << snd_pcm_format_description(format) << std::endl;

	snd_pcm_subformat_t subformat;
	snd_pcm_hw_params_get_subformat(params, &subformat);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Subformat: " <<
		snd_pcm_subformat_name(subformat) << ", " << snd_pcm_subformat_description(subformat) << std::endl;

	unsigned int rate;
	snd_pcm_hw_params_get_rate(params, &rate, &dir);
	_rate.store(rate);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Sample rate: " <<
		rate << " bps" << std::endl;

	unsigned int captureChannelsCount;
	snd_pcm_hw_params_get_channels(params, &captureChannelsCount);
	_captureChannelsCount.store(captureChannelsCount);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Capture channels: " <<
		captureChannelsCount << std::endl;

	unsigned int periodTime;
	snd_pcm_hw_params_get_period_time(params, &periodTime, &dir);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Period time: " <<
	       periodTime << " us" << std::endl;

	snd_pcm_uframes_t framesInPeriod;
	snd_pcm_hw_params_get_period_size(params, &framesInPeriod, &dir);
	_framesInPeriod.store(framesInPeriod);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Period size: " <<
		framesInPeriod << " frames" << std::endl;

	unsigned int bufferTime;
	snd_pcm_hw_params_get_buffer_time(params, &bufferTime, &dir);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Buffer time: " <<
		bufferTime << " us" << std::endl;

	// TODO: Correct bytes per sample calculation
	std::size_t bytesPerSample = 2;
	if (format == SND_PCM_FORMAT_S32_LE) {
		bytesPerSample = 4;
	} else if (format != SND_PCM_FORMAT_S16_LE) {
		std::ostringstream msg;
		msg << "Unsupported format: " << snd_pcm_format_name(format) << '(' <<
			snd_pcm_format_description(format) << ')';
		std::cerr << "ERROR: AudioProcessor::AudioProcessor('" << device << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}
	_bytesPerSample.store(bytesPerSample);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Bytes per sample: " <<
		bytesPerSample << std::endl;

	std::size_t periodBufferSize = framesInPeriod * captureChannelsCount * bytesPerSample;
	_periodBufferSize.store(periodBufferSize);
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Period buffer size: " <<
		periodBufferSize << std::endl;

	std::size_t bufferSize = (periodBufferSize + sizeof(std::size_t)) * PeriodsInBuffer;
	std::clog << "NOTICE: AudioProcessor::AudioProcessor('" << device << "'): Allocating " <<
		(bufferSize / 1024U) << " KB (" << bufferSize <<
		" bytes) capture ring buffer for " << PeriodsInBuffer <<
		" periods (~" << (periodTime * PeriodsInBuffer / 1000000.0) << " sec)" << std::endl;
	_captureRingBuffer.resize(bufferSize);

	// Recording channels initialization
	_captureChannels.resize(captureChannelsCount);
	for (std::size_t i = 0; i < _captureChannels.size(); ++i) {
		_captureChannels[i] = new CaptureChannel(rate, format);
	}

	// Starting capture & record threads
	_state = CaptureStartingState;
	_captureThread = boost::thread(boost::bind(&AudioProcessor::runCapture, this));
	_capturePostProcessingThread = boost::thread(boost::bind(&AudioProcessor::runCapturePostProcessing, this));
}

AudioProcessor::~AudioProcessor()
{
	// Stopping threads
	boost::unique_lock<boost::mutex> lock(_captureMutex);
	_state = IdleState;
	_captureCond.notify_all();
	lock.unlock();

	_captureThread.join();
	_capturePostProcessingThread.join();

	// Closing ALSA
	if (_handle > 0) {
		int rc = snd_pcm_drain(_handle);
		if (rc < 0) {
			std::cerr << "ERROR: AudioProcessor::~AudioProcessor(): Stopping '" << _device <<
				"' PCM device error: " << snd_strerror(rc) << std::endl;
		}
		rc = snd_pcm_close(_handle);
		if (rc < 0) {
			std::cerr << "ERROR: AudioProcessor::~AudioProcessor(): Closing '" << _device <<
				"' PCM device error: " << snd_strerror(rc) << std::endl;
		}
	}

	// Disposing capture channels
	for (std::size_t i = 0; i < _captureChannels.size(); ++i) {
		delete _captureChannels[i];
	}
}

float AudioProcessor::getCaptureLevel(unsigned int channel, std::size_t after, std::size_t end)
{
	_captureChannels[channel]->getLevel(after, end);
}

time_t AudioProcessor::startRecord(const std::string& location)
{
	// TODO: Check destination directory is writable.
	boost::filesystem::file_status s = boost::filesystem::status(location);
	if (s.type() != boost::filesystem::directory_file) {
		std::ostringstream msg;
		msg << "Output location '" << location << "' is not a directory";
		std::cerr << "ERROR: AudioProcessor::startRecord('" << location << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}

	time_t recordTs = time(0);
	assert(recordTs > 0);

	boost::unique_lock<boost::mutex> lock(_captureMutex);
	if (_state != CaptureState) {
		std::ostringstream msg;
		msg << "Invalid recording state: " << _state;
		std::cerr << "ERROR: AudioProcessor::startRecord('" << location << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}
	_state = RecordRequestedState;
	_filesLocation = location;
	_recordTs = recordTs;
	_captureCond.notify_all();

	// Awaiting for state to become 'RecordState'
	while (true) {
		if (_state == RecordState) {
			return recordTs;
		}
		if ((_state != RecordRequestedState) && (_state != RecordRequestConfirmedState)) {
			std::ostringstream msg;
			msg << "Invalid recording state: " << _state;
			std::cerr << "ERROR: AudioProcessor::startRecord('" << location << "'): " << msg.str() << std::endl;
			throw std::runtime_error(msg.str());
		}
		_captureCond.wait(lock);
	}
}

void AudioProcessor::stopRecord()
{
	boost::unique_lock<boost::mutex> lock(_captureMutex);
	if (_state != RecordState) {
		std::ostringstream msg;
		msg << "Invalid recording state: " << _state;
		std::cerr << "ERROR: AudioProcessor::stopRecord(): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}
	_state = CaptureRequestedState;
	_captureCond.notify_all();

	// TODO: Awaiting for state to become 'CaptureState'
	while (true) {
		if (_state == CaptureState) {
			return;
		}
		if ((_state != CaptureRequestedState) && (_state != CaptureRequestConfirmedState)) {
			std::ostringstream msg;
			msg << "Invalid recording state: " << _state;
			std::cerr << "ERROR: AudioProcessor::stopRecord(): " << msg.str() << std::endl;
			throw std::runtime_error(msg.str());
		}
		_captureCond.wait(lock);
	}
}

void AudioProcessor::runCapture()
{
	std::size_t framesInPeriod = _framesInPeriod.load();
	std::size_t periodBufferSize = _periodBufferSize.load();

	assert(framesInPeriod > 0U);
	assert(periodBufferSize > 0U);

	std::size_t periodsCaptured = 0U;
	std::size_t periodsProcessed = 0U;
	char * ringBufferPtr = &_captureRingBuffer.front();

	while (true) {
		std::size_t ringBufferOffset = periodsCaptured % PeriodsInBuffer *
				(periodBufferSize + sizeof(std::size_t));
		// TODO: Check ring buffer overrun!
		int rc = snd_pcm_readi(_handle, ringBufferPtr + ringBufferOffset + sizeof(std::size_t), framesInPeriod);

		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			std::cerr << "ERROR: AudioProcessor::runCapture(): ALSA overrun occurred" << std::endl;
			snd_pcm_prepare(_handle);
			continue;
		} else if (rc < 0) {
			std::cerr << "ERROR: AudioProcessor::runCapture(): ALSA reading data error: " << snd_strerror(rc) << std::endl;
			return;
		} else if (rc != framesInPeriod) {
			// TODO: Check PCM state and exit if not RUNNING?
		}
		*reinterpret_cast<std::size_t *>(ringBufferPtr + ringBufferOffset) = static_cast<std::size_t>(rc);

		//std::clog << '.';

		++periodsCaptured;

		boost::unique_lock<boost::mutex> lock(_captureMutex);
		if (_state == IdleState) {
			break;
		} else if (_state == CaptureStartingState) {
			_state = CaptureState;
		} else if ((_state != CaptureState) && (_state != RecordRequestedState) &&
				(_state != RecordRequestConfirmedState) && (_state != RecordState) &&
				(_state != CaptureRequestedState) && (_state != CaptureRequestConfirmedState)) {
			std::ostringstream msg;
			msg << "Invalid sound processor state: " << _state;
			std::cerr << "ERROR: AudioProcessor::runCapture(): " << msg.str() << std::endl;
			throw std::runtime_error(msg.str());
		}
		_periodsCaptured = periodsCaptured;
		periodsProcessed = _periodsProcessed;
		_captureCond.notify_all();
	}
}

void AudioProcessor::runCapturePostProcessing()
{
	snd_pcm_format_t format = _format.load();
	unsigned int rate = _rate.load();
	unsigned int bytesPerSample = _bytesPerSample.load();
	unsigned int captureChannelsCount = _captureChannelsCount.load();
	unsigned int framesInPeriod = _framesInPeriod.load();
	unsigned int periodBufferSize = _periodBufferSize.load();

	assert(rate > 0U);
	assert(bytesPerSample > 0U);
	assert(captureChannelsCount > 0U);
	assert(framesInPeriod > 0U);
	assert(periodBufferSize > 0U);

	char * ringBufferPtr = &_captureRingBuffer.front();
	Buffer recordBuffer(periodBufferSize);
	char * recordBufferPtr = &recordBuffer.front();

	std::size_t periodsCaptured;
	std::size_t periodsProcessed = 0U;

	bool isRecording = false;
	std::string filesLocation;
	time_t recordTs;

	while (true) {
		bool shouldCreateFiles = false;
		bool shouldCloseFiles = false;

		// Waiting for data to be captured
		boost::unique_lock<boost::mutex> lock(_captureMutex);
		_periodsProcessed = periodsProcessed;
		while (_periodsCaptured <= _periodsProcessed) {
			assert(_periodsCaptured == _periodsCaptured);

			if (_state == IdleState) {
				return;
			} else if (_state == RecordRequestedState) {
				assert(!isRecording);
				assert(!shouldCloseFiles);

				_state = RecordRequestConfirmedState;
				shouldCreateFiles = true;
				isRecording = true;
				filesLocation = _filesLocation;
				recordTs = _recordTs;
			} else if (_state == RecordRequestConfirmedState) {
				_state = RecordState;
				_captureCond.notify_all();
			} else if (_state == CaptureRequestedState) {
				assert(isRecording);
				assert(!shouldCreateFiles);

				_state = CaptureRequestConfirmedState;
				shouldCloseFiles = true;
				isRecording = false;
			} else if (_state == CaptureRequestConfirmedState) {
				_state = CaptureState;
				_captureCond.notify_all();
			} else if ((_state != CaptureStartingState) && (_state != CaptureState) && (_state != RecordState)) {
				std::ostringstream msg;
				msg << "Invalid sound processor state: " << _state;
				std::cerr << "ERROR: AudioProcessor::runCapturePostProcessing(): " << msg.str() << std::endl;
				throw std::runtime_error(msg.str());
			}

			_captureCond.wait(lock);
		}
		periodsCaptured = _periodsCaptured;
		lock.unlock();

		// Creating WAV-files if needed
		if (shouldCreateFiles) {
			assert(!shouldCloseFiles);

			_recordStartedPeriod.store(periodsProcessed + 1U);

			std::clog << "NOTICE: AudioProcessor::runCapturePostProcessing(): Start recording command received" << std::endl;
			for (std::size_t i = 0; i < captureChannelsCount; ++i) {
				std::ostringstream filename;
				filename << "record_" << recordTs << ".track_" <<
					std::setfill('0') << std::setw(2) << (i + 1) << ".wav";
				boost::filesystem::path fullPath = boost::filesystem::path(filesLocation) /
					boost::filesystem::path(filename.str());

				_captureChannels[i]->openFile(fullPath.native());
			}
		}

		// Closing WAV-files if needed
		if (shouldCloseFiles) {
			assert(!shouldCreateFiles);

			_recordFinishedPeriod.store(periodsProcessed);

			std::clog << "NOTICE: AudioProcessor::runCapturePostProcessing(): Stop recording command received" << std::endl;
			for (std::size_t i = 0; i < captureChannelsCount; ++i) {
				_captureChannels[i]->closeFile();
			}
		}

		// Writing captured data
		while (periodsProcessed < periodsCaptured) {
			std::size_t ringBufferOffset = periodsProcessed % PeriodsInBuffer *
					(periodBufferSize + sizeof(std::size_t));
			std::size_t framesCaptured = *reinterpret_cast<std::size_t *>(ringBufferPtr + ringBufferOffset);
			if (framesCaptured != framesInPeriod) {
				std::clog << "WARNING: AudioProcessor::runCapturePostProcessing(): ALSA short read: " <<
						framesCaptured << "/" << framesInPeriod << " frames" << std::endl;
			}

			// Copying data to record buffer
			for (std::size_t i = 0; i < (framesCaptured * captureChannelsCount * bytesPerSample); i += bytesPerSample) {
				std::size_t frame = i / bytesPerSample / captureChannelsCount;
				std::size_t channel = i / bytesPerSample % captureChannelsCount;
				std::size_t j = (frame + channel * framesInPeriod) * bytesPerSample;

				switch (bytesPerSample) {
					case 2U:
						*reinterpret_cast<int16_t *>(recordBufferPtr + j) =
							*reinterpret_cast<int16_t *>(ringBufferPtr + ringBufferOffset + i + sizeof(std::size_t));
						break;
					case 4U:
						*reinterpret_cast<int32_t *>(recordBufferPtr + j) =
							*reinterpret_cast<int32_t *>(ringBufferPtr + ringBufferOffset + i + sizeof(std::size_t));
						break;
					default:
						std::ostringstream msg;
						msg << "Bytes per sample (" << bytesPerSample << ") not supported";
						throw std::runtime_error(msg.str());
				}

			}

			++periodsProcessed;
			for (std::size_t i = 0U; i < captureChannelsCount; ++i) {
				// Extracting WAV-data for channel
				char * buf = recordBufferPtr + i * framesInPeriod * bytesPerSample;
				std::size_t size = framesCaptured * bytesPerSample;

				_capturedPeriods.store(periodsProcessed);

				// Updating level
				_captureChannels[i]->addLevel(periodsProcessed, buf, size);

				if (isRecording) {
					// Writing data to WAV-file
					_captureChannels[i]->write(buf, size);
				}
			}

			//std::clog << '+';
		}
	}
}
