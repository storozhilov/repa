#ifndef __AREPA__MAIN_WINDOW_H
#define __AREPA__MAIN_WINDOW_H

#include "../AudioProcessor.h"
#include <gtkmm.h>

class WaveForm;

class MainWindow : public Gtk::Window
{
public:
	MainWindow(AudioProcessor& audioProcessor, const Glib::ustring& outputPath);
	virtual ~MainWindow();
private:
	enum Const {
		LevelRefreshIntervalMs = 100U,
		WaveFormRefreshIntervalMs = 1000U
	};

	typedef std::vector<Gtk::ProgressBar *> LevelIndicators;
	typedef std::vector<WaveForm *> WaveForms;
	struct Recording {
		Recording(unsigned int channelsCount, std::size_t started) :
			waveForms(channelsCount),
			started(started),
			finished(0U)
		{}

		WaveForms waveForms;
		std::size_t started;
		std::size_t finished;
	};
	typedef std::list<Recording *> Recordings;

	void on_record_button_clicked();

	bool on_level_polling_timeout();
	bool on_waveforms_update_timeout();

	AudioProcessor& _audioProcessor;
	const Glib::ustring _outputPath;

	bool _isRecording;
	std::size_t _recordingStartedFrame;
	std::size_t _recordingFinishedFrame;
	std::size_t _recordingLevelExposedFrame;
	std::size_t _volumeScannedFrame;

	Gtk::VBox _vbox;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _recordButton;
	LevelIndicators _levelIndicators;
	std::vector<Gtk::HBox *> _channelsHBoxes;
	Recordings _recordings;
};

#endif
