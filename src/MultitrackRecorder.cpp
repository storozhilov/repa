#include "MultitrackRecorder.h"

MultitrackRecorder::MultitrackRecorder() :
	_shouldRun(false),
	_captureThread(),
	_recordThread()
{}

void MultitrackRecorder::start(const std::string& location, const std::string& device)
{
	_shouldRun = true;
	_captureThread = boost::thread(boost::bind(&MultitrackRecorder::runCapture, this));
	_recordThread = boost::thread(boost::bind(&MultitrackRecorder::runRecord, this));
}

void MultitrackRecorder::stop()
{
	_shouldRun = false;
	_captureThread.join();
}

void MultitrackRecorder::runCapture()
{
	while (_shouldRun) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}

void MultitrackRecorder::runRecord()
{
	while (_shouldRun) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}
