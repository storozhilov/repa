#include <iostream>
#include <csignal>
#include <unistd.h>
#include <ctime>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>

#include "../AudioProcessor.h"

namespace
{
	volatile static std::sig_atomic_t stopped = static_cast<std::sig_atomic_t>(false);
}

void signal_handler(int signal)
{
	stopped = static_cast<std::sig_atomic_t>(true);
}

int main(int argc, char * argv[]) {
	std::string device;
	std::string location;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "Produce help message")
		("device,D", boost::program_options::value<std::string>(&device)->default_value("default"), "ALSA capture device")
		("output,O", boost::program_options::value<std::string>(&location)->default_value(std::string(getcwd(NULL, 0))),
		 "Location of the output")
	;

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 0;
	}

	std::cout << "Recording audio data from '" << device << "' ALSA capture device to '" << location << "' location" << std::endl;
	std::signal(SIGTERM, signal_handler);
	std::signal(SIGINT, signal_handler);

	AudioProcessor ap(device.c_str());

	std::ostringstream filenamePrefix;
	filenamePrefix << "record_" << time(0) << '.';
	ap.startRecord(location.c_str(), filenamePrefix.str().c_str());

	while (!static_cast<bool>(stopped)) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	std::cout << "Termination command received -> exiting" << std::endl;
	ap.stopRecord();
	return 0;
}
