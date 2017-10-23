#include "MainWindow.h"
#include <iostream>
#include <gdk/gdkx.h>

MainWindow::MainWindow(SourceUris& sourceUris) :
	Gtk::Window(),
	_vbox(false, 6),
	_mainVideoArea(),
	_sourcesBox(false, 6),
	_buttonBox(),
	_firstSourceButton("First"),
	_secondSourceButton("Second"),
	_sourceVideoAreas(),
	_sourceUris(sourceUris),
	_sourceHandles(sourceUris.size()),
	_mainVideoAreaWindowHandle(0),
	_videoProcessor()
{
	add(_vbox);
	_vbox.pack_start(_mainVideoArea, Gtk::PACK_EXPAND_WIDGET);
	_vbox.pack_start(_sourcesBox, Gtk::PACK_SHRINK);
	_vbox.pack_start(_buttonBox, Gtk::PACK_SHRINK);

	_buttonBox.pack_start(_firstSourceButton);
	_buttonBox.pack_start(_secondSourceButton);

	_firstSourceButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_first_button_clicked));
	_secondSourceButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_second_button_clicked));

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

	for (std::size_t i = 0; i < _sourceUris.size(); ++i) {
		_sourceHandles[i] = _videoProcessor->addSource(_sourceUris[i].c_str());
	}

	_videoProcessor->start();
	std::cout << "Video processor started" << std::endl;
}

void MainWindow::on_first_button_clicked()
{
	_videoProcessor->switchSource(_sourceHandles[0]);
}

void MainWindow::on_second_button_clicked()
{
	_videoProcessor->switchSource(_sourceHandles[1]);
}
