#include <iostream>
#include <csignal>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include "MultitrackRecorder.h"

namespace
{
	volatile static std::sig_atomic_t stopped = static_cast<std::sig_atomic_t>(false);
}

void signal_handler(int signal)
{
	stopped = static_cast<std::sig_atomic_t>(true);
}

int main(int argc, char * argv[]) {
	std::signal(SIGTERM, signal_handler);

	std::cout << "Starting multitrack recorder" << std::endl;
	MultitrackRecorder recorder;
	recorder.start();
	while (!static_cast<bool>(stopped)) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
	std::cout << "SIGTERM received -> exiting" << std::endl;
	recorder.stop();
	return 0;
}
