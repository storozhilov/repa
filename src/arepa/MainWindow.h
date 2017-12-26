#ifndef __AREPA__MAIN_WINDOW_H
#define __AREPA__MAIN_WINDOW_H

#include "AudioProcessor.h"
#include <gtkmm.h>

class MainWindow : public Gtk::Window
{
public:

	MainWindow(AudioProcessor& audioProcessor, const Glib::ustring& outputPath);
private:
	typedef std::vector<Gtk::ProgressBar *> LevelIndicators;

	void on_record_button_clicked();

	AudioProcessor& _audioProcessor;
	const Glib::ustring _outputPath;

	bool _isRecording;

	Gtk::VBox _vbox;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _recordButton;
	LevelIndicators _levelIndicators;
};

#endif
