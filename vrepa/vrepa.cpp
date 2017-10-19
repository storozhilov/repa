#include <iostream>
#include <gtkmm.h>
#include <vector>

#include "VideoProcessor.h"
#include "MainWindow.h"

int main(int argc, char * argv[]) {
	Gtk::Main kit(argc, argv);
	Gst::init(argc, argv);

	guint major, minor, micro, nano;
	Gst::version(major, minor, micro, nano);

	std::cout << "GStreamer version: " << major << "." << minor << "." << micro << "." << nano << std::endl;

	VideoProcessor vp;
	std::vector<VideoProcessor::SourceHandle> sources(2);

	sources[0] = vp.addSource("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream");
	sources[1] = vp.addSource("rtsp://192.168.1.3:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream");

	vp.start();
	std::cout << "Video processor started" << std::endl;

	MainWindow window(vp);
	Gtk::Main::run(window);

	vp.stop();
	std::cout << "Video processor stopped" << std::endl;
}
