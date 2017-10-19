#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(VideoProcessor& vp) :
	Gtk::Window(),
	_vbox(false, 6)
{}

bool MainWindow::on_delete_event(GdkEventAny * event)
{
	std::cout << "Main window closed" << std::endl;
	return false;
}
