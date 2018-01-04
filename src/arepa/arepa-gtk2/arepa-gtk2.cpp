#include "../AudioProcessor.h"
#include "MainWindow.h"

#include <iostream>
#include <glibmm.h>
#include <gtkmm.h>

class OptionGroup : public Glib::OptionGroup
{
public:
	OptionGroup() :
		Glib::OptionGroup("arepa_group", "Audio repa options group", "Help about audio repa options group"),
		captureDevice("hw")
	{
		Glib::OptionEntry captureDeviceOptionEntry;
		captureDeviceOptionEntry.set_long_name("capture-device");
		captureDeviceOptionEntry.set_short_name('C');
		captureDeviceOptionEntry.set_description("Capture device name (optional, default: 'hw')");
		add_entry(captureDeviceOptionEntry, captureDevice);

		Glib::OptionEntry sessionLocationOptionEntry;
		sessionLocationOptionEntry.set_long_name("session-location");
		sessionLocationOptionEntry.set_short_name('l');
		sessionLocationOptionEntry.set_description("Session location (required)");
		add_entry(sessionLocationOptionEntry, sessionLocation);
	}

	Glib::ustring captureDevice;
	Glib::ustring sessionLocation;
};

int main(int argc, char * argv[]) {

	Glib::init();
	Glib::OptionContext oc;
	OptionGroup og;
	oc.set_main_group(og);
	Gtk::Main kit(argc, argv, oc);

	if (og.sessionLocation.empty()) {
		std::cerr << "ERROR: Empty session location." << std::endl <<
			oc.get_help() << std::endl;
		return 1;
	}

	std::clog << "NOTICE: Capture device: '" << og.captureDevice << '\'' << std::endl;
	std::clog << "NOTICE: Session location: '" << og.sessionLocation << '\'' << std::endl;

	AudioProcessor ap(og.captureDevice.c_str());
	MainWindow mw(ap, og.sessionLocation);
	Gtk::Main::run(mw);
}
