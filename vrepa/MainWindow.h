#ifndef __VREPA__MAIN_WINDOW_H
#define __VREPA__MAIN_WINDOW_H

#include "VideoProcessor.h"
#include <gtkmm.h>
#include <memory>

class MainWindow : public Gtk::Window
{
public:
	MainWindow();
protected:
	Gtk::VBox _vbox;
	Gtk::DrawingArea _mainVideoArea;
	Gtk::HBox _hbox;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _firstSourceButton;
	Gtk::Button _secondSourceButton;

	VideoProcessor::SourceHandle _firstSourceHandle;
	VideoProcessor::SourceHandle _secondSourceHandle;

	void on_first_button_clicked();
	void on_second_button_clicked();

	virtual bool on_delete_event(GdkEventAny * event);
private:
	guintptr _mainVideoAreaWindowHandle;
	std::unique_ptr<VideoProcessor> _videoProcessor;

	void on_main_video_area_realize();

	friend class VideoProcessor;
};

#endif
