#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

#include <vector>

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
	std::vector<VideoProcessor::SourceHandle> sources(2);

	sources[0] = vp.addSource("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream");
	sources[1] = vp.addSource("rtsp://192.168.1.3:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream");

	vp.start();
	std::cout << "Video processor started" << std::endl;

	std::size_t counter = 0;
	while (!static_cast<bool>(stopped)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::size_t sourceIndex = counter++ % 2;
		vp.switchSource(sources[sourceIndex]);
	}
	std::cout << "Termination command received -> exiting" << std::endl;
	vp.stop();
	std::cout << "Video processor stopped" << std::endl;
}
