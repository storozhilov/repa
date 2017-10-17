#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

#include "VideoProcessor.h"

namespace
{
	volatile static std::sig_atomic_t stopped = static_cast<std::sig_atomic_t>(false);
}

void signal_handler(int signal)
{
	stopped = static_cast<std::sig_atomic_t>(true);
}

int main(int argc, char * argv[]) {
	Gst::init(argc, argv);

	guint major, minor, micro, nano;
	Gst::version(major, minor, micro, nano);

	std::cout << "GStreamer version: " << major << "." << minor << "." << micro << "." << nano << std::endl;

	std::signal(SIGTERM, signal_handler);
	std::signal(SIGINT, signal_handler);

	VideoProcessor vp;
	vp.start();
	std::cout << "Video processor started" << std::endl;

	while (!static_cast<bool>(stopped)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		vp.switchSource(0);
	}
	std::cout << "Termination command received -> exiting" << std::endl;
	vp.stop();
	std::cout << "Video processor stopped" << std::endl;
}
