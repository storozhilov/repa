#ifndef __SR__MULTITRACK_RECORDER_H
#define __SR__MULTITRACK_RECORDER_H

#include <boost/atomic.hpp>
#include <boost/thread.hpp>

class MultitrackRecorder
{
public:
	MultitrackRecorder();

	void start(const std::string& location, const std::string& device);
	void stop();
private:
	void runCapture();
	void runRecord();

	boost::atomic<bool> _shouldRun;
	boost::thread _captureThread;
	boost::thread _recordThread;
};

#endif
