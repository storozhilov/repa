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
	Gtk::DrawingArea _mainVideoArea;
	Gtk::HBox _hbox;

	virtual bool on_delete_event(GdkEventAny * event);
private:
	guintptr _videoWindowHandle;

	void on_main_video_area_realize();
};

#endif
