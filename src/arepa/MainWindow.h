#ifndef __AREPA__MAIN_WINDOW_H
#define __AREPA__MAIN_WINDOW_H

#include "AudioProcessor.h"
#include <gtkmm.h>

class WaveForm;

class MainWindow : public Gtk::Window
{
public:

	MainWindow(AudioProcessor& audioProcessor, const Glib::ustring& outputPath);
private:
	enum Const {
		LevelRefreshIntervalMs = 100U
	};

	typedef std::vector<Gtk::ProgressBar *> LevelIndicators;
	typedef std::vector<WaveForm *> WaveForms;

	void on_record_button_clicked();

	bool on_level_polling_timeout();

	AudioProcessor& _audioProcessor;
	const Glib::ustring _outputPath;

	bool _isRecording;

	Gtk::VBox _vbox;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _recordButton;
	LevelIndicators _levelIndicators;
	WaveForms _waveForms;
};

#endif
