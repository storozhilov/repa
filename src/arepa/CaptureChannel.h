#ifndef __AREPA__CAPTURE_CHANNEL_H
#define __AREPA__CAPTURE_CHANNEL_H

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <sndfile.hh>
#include <boost/atomic.hpp>

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

#endif
