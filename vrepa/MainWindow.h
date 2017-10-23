#ifndef __VREPA__MAIN_WINDOW_H
#define __VREPA__MAIN_WINDOW_H

#include "VideoProcessor.h"
#include <gtkmm.h>
#include <memory>
#include <vector>

class MainWindow : public Gtk::Window
{
public:
	typedef std::vector<std::string> SourceUris;

	MainWindow(SourceUris& sourceUris);
protected:
	typedef std::vector<Glib::RefPtr<Gtk::DrawingArea>> SourceVideoAreas;

	Gtk::VBox _vbox;
	Gtk::DrawingArea _mainVideoArea;
	Gtk::HBox _sourcesBox;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _firstSourceButton;
	Gtk::Button _secondSourceButton;
	SourceVideoAreas _sourceVideoAreas;

	void on_first_button_clicked();
	void on_second_button_clicked();

	virtual bool on_delete_event(GdkEventAny * event);
private:
	typedef std::vector<VideoProcessor::SourceHandle> SourceHandles;

	SourceUris _sourceUris;
	SourceHandles _sourceHandles;
	guintptr _mainVideoAreaWindowHandle;
	std::unique_ptr<VideoProcessor> _videoProcessor;

	void on_main_video_area_realize();

	friend class VideoProcessor;
};

#endif
