#include "CaptureChannel.h"
#include <iostream>
#include <sstream>

CaptureChannel::CaptureChannel(unsigned int rate, snd_pcm_format_t alsaFormat) :
	_rate(rate),
	_alsaFormat(alsaFormat),
	_sfFormat(SF_FORMAT_WAV),
	_level(0U),
	_filename(),
	_file(0)
{
	switch (_alsaFormat) {
		case SND_PCM_FORMAT_S16_LE:
			_sfFormat |= SF_FORMAT_PCM_16;
			break;
		case SND_PCM_FORMAT_S32_LE:
			_sfFormat |= SF_FORMAT_PCM_32;
			break;
		case SND_PCM_FORMAT_FLOAT_LE:
			_sfFormat |= SF_FORMAT_FLOAT;
			break;
		default:
			std::ostringstream msg;
			msg << "WAV-file format is not supported: " << snd_pcm_format_name(_alsaFormat) <<
				", " << snd_pcm_format_description(_alsaFormat);
			std::cerr << "ERROR: CaptureChannel::CaptureChannel(): " << msg.str() << std::endl;
			throw std::runtime_error(msg.str());
	}
}

CaptureChannel::~CaptureChannel()
{
	if (fileIsOpen()) {
		std::cerr << "WARNING: CaptureChannel::~CaptureChannel(): File was not closed explicitly: '" <<
			_filename << "' -> closing" << std::endl;
		closeFile();
	}
}

void CaptureChannel::openFile(const std::string& filename)
{
	if (fileIsOpen()) {
		std::ostringstream msg;
		msg << "File is already opened: '" << _filename << '\'';
		std::cerr << "ERROR: CaptureChannel::openFile('" << filename << "'): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}
	std::cout << "NOTICE: CaptureChannel::openFile('" << filename << "'): Opening WAV-file" << std::endl;
	_file = new SndfileHandle(filename.c_str(), SFM_WRITE, _sfFormat, 1, _rate);
	_filename = filename;
}

void CaptureChannel::closeFile()
{
	if (!fileIsOpen()) {
		std::ostringstream msg;
		msg << "File is already closed: '" << _filename << '\'';
		std::cerr << "ERROR: CaptureChannel::closeFile(): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}
	std::cout << "NOTICE: CaptureChannel::closeFile(): Closing '" << _filename << "' WAV-file" << std::endl;
	delete _file;
	_file = 0;
}

void CaptureChannel::write(const char * buf, std::size_t size)
{
	sf_count_t itemsToWrite = 0;
	sf_count_t itemsWritten = 0;

	switch (_alsaFormat) {
		case SND_PCM_FORMAT_S16_LE:
			itemsToWrite = size / 2;
			itemsWritten = _file->write(reinterpret_cast<const short *>(buf), itemsToWrite);
			break;
		case SND_PCM_FORMAT_S32_LE:
			itemsToWrite = size / 4;
			itemsWritten = _file->write(reinterpret_cast<const int *>(buf), itemsToWrite);
			break;
		case SND_PCM_FORMAT_FLOAT_LE:
			itemsToWrite = size / 4;
			itemsWritten = _file->write(reinterpret_cast<const float *>(buf), itemsToWrite);
			break;
		default:
			std::ostringstream msg;
			msg << "WAV-file format is not supported: " << snd_pcm_format_name(_alsaFormat) <<
				", " << snd_pcm_format_description(_alsaFormat);
			std::cerr << "CaptureChannel::write(): ERROR: " << msg.str() << std::endl;
			throw std::runtime_error(msg.str());
	}

	if (itemsWritten != itemsToWrite) {
		std::ostringstream msg;
		msg << "Unconsistent written items count: " << itemsWritten << '/' << itemsToWrite;
		std::cerr << "ERROR: CaptureChannel::write(): " << msg.str() << std::endl;
		throw std::runtime_error(msg.str());
	}
}