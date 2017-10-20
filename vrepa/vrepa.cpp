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

	MainWindow mainWindow;
	Gtk::Main::run(mainWindow);
}
