#ifndef __VREPA__MAIN_WINDOW_H
#define __VREPA__MAIN_WINDOW_H

#include "VideoProcessor.h"
#include <gtkmm.h>
#include <memory>
#include <vector>
#include <map>

class MainWindow : public Gtk::Window
{
public:
	typedef std::vector<std::string> SourceUris;

	MainWindow(SourceUris& sourceUris);
protected:
	struct SourceData {
		std::shared_ptr<Gtk::DrawingArea> videoArea;
		guintptr videoAreaWindowHandle;
	};

	typedef std::map<VideoProcessor::SourceHandle, SourceData> SourcesMap;

	Gtk::VBox _vbox;
	Gtk::DrawingArea _mainVideoArea;
	Gtk::HBox _sourcesBox;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _firstSourceButton;
	Gtk::Button _secondSourceButton;
	SourcesMap _sourcesMap;

	void on_first_button_clicked();
	void on_second_button_clicked();

	virtual bool on_delete_event(GdkEventAny * event);
private:
	SourceUris _sourceUris;
	guintptr _mainVideoAreaWindowHandle;
	std::unique_ptr<VideoProcessor> _videoProcessor;

	void on_main_video_area_realize();
	void on_source_video_area_realize(VideoProcessor::SourceHandle);

	friend class VideoProcessor;
};

#endif
