#ifndef __SR__MULTITRACK_RECORDER_H
#define __SR__MULTITRACK_RECORDER_H

#include <boost/atomic.hpp>
#include <boost/thread.hpp>

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const char * location = 0, const char * device = 0);
	void stop();
private:
	void runCapture();
	void runRecord();

	boost::atomic<bool> _shouldRun;
	boost::thread _captureThread;
	boost::thread _recordThread;
};

#endif
