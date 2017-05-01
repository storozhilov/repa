#include "MultitrackRecorder.h"

#include <stdexcept>
#include <sstream>
#include <iostream>

MultitrackRecorder::MultitrackRecorder() :
	_shouldRun(false),
	_captureThread(),
	_recordThread(),
	_captureHandle(),
	_access(),
	_format(),
	_subformat(),
	_rate(),
	_channels(),
	_periodTime(),
	_periodSize(),
	_bufferTime(),
	_captureBuffer()
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
	int rc = snd_pcm_open(&_captureHandle, device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Opening PCM device error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}


	snd_pcm_hw_params_t * params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(_captureHandle, params);

	rc = snd_pcm_hw_params_set_access(_captureHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting access type error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params_set_format(_captureHandle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0) {
		std::cerr << "WARNING: Requesting format error: " << snd_strerror(rc) <<std::endl;
	}

	rc = snd_pcm_hw_params_set_channels(_captureHandle, params, 2);
	if (rc < 0) {
		std::cerr << "WARNING: Requesting channels amount error: " << snd_strerror(rc) <<std::endl;
	}

	unsigned int val;
	int dir;

	val = 44100;
	rc = snd_pcm_hw_params_set_rate_near(_captureHandle, params, &val, &dir);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Restricting sample rate error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	rc = snd_pcm_hw_params(_captureHandle, params);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Setting H/W parameters error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}

	std::cout << "PCM handle name: " << snd_pcm_name(_captureHandle) << std::endl <<
		"PCM state: " << snd_pcm_state_name(snd_pcm_state(_captureHandle)) << std::endl;

	snd_pcm_hw_params_get_access(params, &_access);
	std::cout << "Access type: " << snd_pcm_access_name(_access) << std::endl;

	snd_pcm_hw_params_get_format(params, &_format);
	std::cout << "Format: " << snd_pcm_format_name(_format) << ", " << snd_pcm_format_description(_format) << std::endl;

	snd_pcm_hw_params_get_subformat(params, &_subformat);
	std::cout << "Subformat: " << snd_pcm_subformat_name(_subformat) << ", " << snd_pcm_subformat_description(_subformat) << std::endl;

	snd_pcm_hw_params_get_rate(params, &_rate, &dir);
	std::cout << "Sample rate: " << _rate << " bps" << std::endl;

	snd_pcm_hw_params_get_channels(params, &_channels);
	std::cout << "Channels: " << _channels << std::endl;

	snd_pcm_hw_params_get_period_time(params, &_periodTime, &dir);
	std::cout << "Period time: " << _periodTime << " us" << std::endl;

	snd_pcm_hw_params_get_period_size(params, &_periodSize, &dir);
	std::cout << "Period size: " << _periodSize << " frames" << std::endl;

	snd_pcm_hw_params_get_buffer_time(params, &_bufferTime, &dir);
	std::cout << "Buffer time: " << _bufferTime << " us" << std::endl;

	// TODO: Sample bytes calculation
	//std::size_t sampleBytes = 2;
	std::size_t sampleBytes = 4;
	unsigned int bufferSize = _periodSize * _channels * sampleBytes;
	std::cout << "Allocating " << bufferSize << " bytes capture buffer" << std::endl;
	_captureBuffer.resize(bufferSize);
	// TODO: Other params

	_shouldRun = true;
	_captureThread = boost::thread(boost::bind(&MultitrackRecorder::runCapture, this));
	_recordThread = boost::thread(boost::bind(&MultitrackRecorder::runRecord, this));
}

void MultitrackRecorder::stop()
{
	_shouldRun = false;
	_captureThread.join();
	_recordThread.join();

	int rc = snd_pcm_drain(_captureHandle);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Stopping PCM device error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}
	rc = snd_pcm_close(_captureHandle);
	if (rc < 0) {
		std::ostringstream msg;
		msg << "Closing PCM device error: " << snd_strerror(rc);
		throw std::runtime_error(msg.str());
	}
}

void MultitrackRecorder::runCapture()
{
	while (_shouldRun) {
		int rc = snd_pcm_readi(_captureHandle, &_captureBuffer[0], _periodSize);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			std::cerr << "ERROR: Overrun occurred" << std::endl;
			snd_pcm_prepare(_captureHandle);
			continue;
		} else if (rc < 0) {
			std::cerr << "ERROR: Reading data error: " << snd_strerror(rc) << std::endl;
			return;
		} else if (rc != _periodSize) {
			std::cerr << "WARNING: Short read, read " << rc << "/" << _periodSize << " frames" << std::endl;
		}
		std::cout << '.';
		//boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}

void MultitrackRecorder::runRecord()
{
	while (_shouldRun) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}
