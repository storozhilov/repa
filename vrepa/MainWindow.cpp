#include "MainWindow.h"
#include <iostream>
#include <gdk/gdkx.h>

MainWindow::MainWindow(VideoProcessor& vp) :
	Gtk::Window(),
	_vbox(false, 6),
	_mainVideoArea(),
	_hbox(false, 6),
	_videoWindowHandle(0)
{
	add(_vbox);
	_vbox.pack_start(_mainVideoArea, Gtk::PACK_EXPAND_WIDGET);
	_vbox.pack_start(_hbox, Gtk::PACK_SHRINK);

	_mainVideoArea.signal_realize().connect(sigc::mem_fun(*this, &MainWindow::on_main_video_area_realize));

	show_all_children();
}

bool MainWindow::on_delete_event(GdkEventAny * event)
{
	std::cout << "Main window closed" << std::endl;
	return false;
}

void MainWindow::on_main_video_area_realize()
{
	gulong xid = GDK_WINDOW_XID (_mainVideoArea.get_window()->gobj());
	_videoWindowHandle = xid;
	std::cout << "Main video area realized, handle: " << _videoWindowHandle << std::endl;

	// TODO: Create a video processor here
}
