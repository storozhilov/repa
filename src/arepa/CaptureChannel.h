#ifndef __AREPA__CAPTURE_CHANNEL_H
#define __AREPA__CAPTURE_CHANNEL_H

#include "Indicator.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <sndfile.hh>
#include <boost/atomic.hpp>

class CaptureChannel
{
public:
	enum Consts {
		IndicatorHistorySize = 1024
	};

	CaptureChannel(unsigned int rate, snd_pcm_format_t alsaFormat);
	~CaptureChannel();

	void addLevel(float level);
	float getLevel(std::size_t ms);

	void openFile(const std::string& filename);
	void closeFile();
	inline bool fileIsOpen() const
	{
		return _file;
	}
	void write(const char * buf, std::size_t size);
private:
	typedef Indicator<float> IndicatorType;

	unsigned int _rate;
	snd_pcm_format_t _alsaFormat;
	int _sfFormat;
	IndicatorType _indicator;
	std::string _filename;
	SndfileHandle * _file;
};

#endif
