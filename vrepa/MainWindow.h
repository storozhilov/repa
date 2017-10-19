#ifndef __VREPA__MAIN_WINDOW_H
#define __VREPA__MAIN_WINDOW_H

#include "VideoProcessor.h"
#include <gtkmm.h>

class MainWindow : public Gtk::Window
{
public:
	MainWindow(VideoProcessor& vp);
protected:
	Gtk::VBox _vbox;

	virtual bool on_delete_event(GdkEventAny * event);
};

#endif
