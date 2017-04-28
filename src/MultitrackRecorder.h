#ifndef __SR__MULTITRACK_RECORDER_H
#define __SR__MULTITRACK_RECORDER_H

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const char * location = 0, const char * device = 0);
	void stop();
};

#endif
