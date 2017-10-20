#include "MainWindow.h"
#include <iostream>
#include <gdk/gdkx.h>

MainWindow::MainWindow() :
	Gtk::Window(),
	_vbox(false, 6),
	_mainVideoArea(),
	_hbox(false, 6),
	_closeButton("Close"),
	_mainVideoAreaWindowHandle(0),
	_videoProcessor()
{
	add(_vbox);
	_vbox.pack_start(_mainVideoArea, Gtk::PACK_EXPAND_WIDGET);
	_vbox.pack_start(_hbox, Gtk::PACK_SHRINK);
	_vbox.pack_start(_closeButton, Gtk::PACK_SHRINK);

	_mainVideoArea.signal_realize().connect(sigc::mem_fun(*this, &MainWindow::on_main_video_area_realize));

	show_all_children();
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

	//sources[0] = vp.addSource("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream");
	//sources[1] = vp.addSource("rtsp://192.168.1.3:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream");
	/*sources[0] = */_videoProcessor->addSource(VideoProcessor::SourceTestSnow);
	/*sources[1] = */_videoProcessor->addSource(VideoProcessor::SourceTestSmpte);

	_videoProcessor->start();
	std::cout << "Video processor started" << std::endl;
}
