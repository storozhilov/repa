#ifndef __AREPA__CAPTURE_CHANNEL_H
#define __AREPA__CAPTURE_CHANNEL_H

#include "Indicator.h"
#include "Diagram.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <sndfile.hh>
#include <boost/atomic.hpp>

class CaptureChannel
{
public:
	enum Consts {
		DiagramHistorySize = 1024
	};

	CaptureChannel(unsigned int rate, snd_pcm_format_t alsaFormat);
	~CaptureChannel();

	float getLevel();

	void openFile(const std::string& filename);
	void closeFile();
	inline bool fileIsOpen() const
	{
		return _file;
	}
	void addLevel(std::size_t periodNumber, const char * buf, std::size_t size);
	void write(const char * buf, std::size_t size);
private:
	typedef Diagram<std::size_t, float> DiagramType;

	unsigned int _rate;
	snd_pcm_format_t _alsaFormat;
	int _sfFormat;
	DiagramType _diagram;
	std::size_t _lastLevelCheckPeriodNumber;
	std::string _filename;
	SndfileHandle * _file;
};

#endif
