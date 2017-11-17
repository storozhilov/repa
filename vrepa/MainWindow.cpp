#include "MainWindow.h"
#include <iostream>
#include <algorithm>
#include <gdk/gdkx.h>

MainWindow::MainWindow(SourceUris& sourceUris) :
	Gtk::Window(),
	_vbox(false, 6),
	_mainVideoArea(),
	_sourcesBox(false, 6),
	_buttonBox(Gtk::BUTTONBOX_START, 6),
	_recordButton("Start recording"),
	_streamingButton("Start streaming"),
	_sourcesMap(),
	_sourceUris(sourceUris),
	_mainVideoAreaWindowHandle(0),
	_videoProcessor(),
	_isRecording(false),
	_isStreaming(false)
{
	add(_vbox);
	_vbox.pack_start(_mainVideoArea, Gtk::PACK_EXPAND_WIDGET);
	_vbox.pack_start(_sourcesBox, Gtk::PACK_SHRINK);
	_vbox.pack_start(_buttonBox, Gtk::PACK_SHRINK);

	_buttonBox.pack_start(_recordButton, Gtk::PACK_SHRINK);
	_buttonBox.pack_start(_streamingButton, Gtk::PACK_SHRINK);

	_recordButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_record_button_clicked));
	_streamingButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_second_button_clicked));

	_mainVideoArea.signal_realize().connect(sigc::mem_fun(*this, &MainWindow::on_main_video_area_realize));

	show_all_children();

	resize(1024, 768);
}

bool MainWindow::on_delete_event(GdkEventAny * event)
{
	std::cout << "Main window closed" << std::endl;

	_videoProcessor->stop();
	std::cout << "Video processor stopped" << std::endl;
	_videoProcessor.reset();
	std::cout << "Video processor destroyed" << std::endl;

	return false;
}

void MainWindow::on_main_video_area_realize()
{
	gulong xid = GDK_WINDOW_XID (_mainVideoArea.get_window()->gobj());
	_mainVideoAreaWindowHandle = xid;
	std::cout << "Main video area realized, handle: " << _mainVideoAreaWindowHandle << std::endl;

	_videoProcessor.reset(new VideoProcessor(*this));
	std::cout << "Video processor created" << std::endl;

	for (auto uri : _sourceUris) {
		auto sourceHandle = _videoProcessor->addSource(uri.c_str());
		SourceData sourceData;

		sourceData.videoArea.reset(new Gtk::DrawingArea());
		sourceData.videoArea->set_events(Gdk::BUTTON_PRESS_MASK);
		sourceData.videoArea->signal_button_press_event().connect(
				sigc::bind(sigc::mem_fun(*this, &MainWindow::on_source_video_area_button_press), sourceHandle));
		sourceData.videoArea->set_size_request(200, 150);
		sourceData.videoArea->signal_realize().connect(
				sigc::bind(sigc::mem_fun(*this, &MainWindow::on_source_video_area_realize), sourceHandle));
		_sourcesBox.pack_start(*sourceData.videoArea.get(), Gtk::PACK_SHRINK);

		_sourcesMap.insert(SourcesMap::value_type(sourceHandle, sourceData));

		sourceData.videoArea->show();
	}
	show_all_children();
}

void MainWindow::on_source_video_area_realize(VideoProcessor::SourceHandle sourceHandle)
{
	gulong xid = GDK_WINDOW_XID (_sourcesMap[sourceHandle].videoArea->get_window()->gobj());
	_sourcesMap[sourceHandle].videoAreaWindowHandle = xid;
	std::cout << "Source video area realized, handle: " << xid << std::endl;

	// Starting video-processor after all video areas are realized
	std::size_t sourceVideoAreasRealized = std::count_if(_sourcesMap.begin(), _sourcesMap.end(),
			[](SourcesMap::value_type& e) { return e.second.videoAreaWindowHandle > 0; });
	if (_sourceUris.size() > sourceVideoAreasRealized) {
		return;
	}
	_videoProcessor->start();
	std::cout << "Video processor started" << std::endl;
}

bool MainWindow::on_source_video_area_button_press(GdkEventButton * event, VideoProcessor::SourceHandle sourceHandle)
{
	std::cout << "Video area clicked for " << sourceHandle << " source" << std::endl;
	_videoProcessor->switchSource(sourceHandle);
	return true;
}

void MainWindow::on_record_button_clicked()
{
	_isRecording = !_isRecording;
	if (_isRecording) {
		_recordButton.set_label("Stop recording");
		std::cout << "<b>Recording started</b>" << std::endl;
	} else {
		_recordButton.set_label("Start recording");
		std::cout << "Recording stopped" << std::endl;
	}
}

void MainWindow::on_second_button_clicked()
{
	_isStreaming = !_isStreaming;
	if (_isStreaming) {
		_streamingButton.set_label("Stop streaming");
		std::cout << "Streaming started" << std::endl;
	} else {
		_streamingButton.set_label("Start streaming");
		std::cout << "Streaming stopped" << std::endl;
	}
}
