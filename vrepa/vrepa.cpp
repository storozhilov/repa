#include <iostream>
#include <gtkmm.h>
#include <vector>
#include <gstreamermm.h>

#include "MainWindow.h"

int main(int argc, char * argv[]) {
	Gtk::Main kit(argc, argv);
	Gst::init(argc, argv);

	guint major, minor, micro, nano;
	Gst::version(major, minor, micro, nano);

	std::cout << "GStreamer version: " << major << "." << minor << "." << micro << "." << nano << std::endl;

	MainWindow::SourceUris uris(2);
	uris[0] = "rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream";
	uris[1] = "rtsp://192.168.1.3:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream";
	//uris[0] = VideoProcessor::SourceTestSmpte100;
	//uris[1] = VideoProcessor::SourceTestCircular;
	//uris[1] = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov";

	MainWindow mainWindow(uris);
	Gtk::Main::run(mainWindow);
}
