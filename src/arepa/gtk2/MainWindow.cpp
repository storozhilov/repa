#include "MainWindow.h"
#include "WaveForm.h"

MainWindow::MainWindow(AudioProcessor& audioProcessor, const Glib::ustring& outputPath) :
	Gtk::Window(),
	_audioProcessor(audioProcessor),
	_outputPath(outputPath),
	_isRecording(false),
	_recordingStartedFrame(0U),
	_recordingFinishedFrame(0U),
	_recordingLevelExposedFrame(0U),
	_volumeScannedFrame(0U),
	_vbox(false, 6),
	_buttonBox(Gtk::BUTTONBOX_START, 6),
	_recordButton("Start recording"),
	_levelIndicators(_audioProcessor.getCaptureChannels()),
	_channelsHBoxes(_audioProcessor.getCaptureChannels()),
	_recordings()
{
	add(_vbox);
	_vbox.pack_start(_buttonBox, Gtk::PACK_SHRINK);
	_buttonBox.pack_start(_recordButton, Gtk::PACK_SHRINK);
	_recordButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_record_button_clicked));

	for (std::size_t i = 0U; i < _audioProcessor.getCaptureChannels(); ++i) {
		_channelsHBoxes[i] = manage(new Gtk::HBox(false, 6));
		_vbox.pack_start(*_channelsHBoxes[i], Gtk::PACK_SHRINK);

		_levelIndicators[i] = manage(new Gtk::ProgressBar());
		_levelIndicators[i]->set_orientation(Gtk::PROGRESS_BOTTOM_TO_TOP);
		_channelsHBoxes[i]->pack_start(*_levelIndicators[i], Gtk::PACK_SHRINK);
	}
	show_all_children();

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_level_polling_timeout),
			LevelRefreshIntervalMs);
}

MainWindow::~MainWindow()
{
	for (auto recording : _recordings) {
		delete recording;
	}
}

void MainWindow::on_record_button_clicked()
{
	_isRecording = !_isRecording;
	if (_isRecording) {
		std::ostringstream filenamePrefix;
		filenamePrefix << "record_" << time(0) << '.';
		_audioProcessor.startRecord(_outputPath.c_str(), filenamePrefix.str().c_str());
		_recordingStartedFrame = _audioProcessor.getRecordStartedFrame();
		_recordingLevelExposedFrame = _recordingStartedFrame - 1U;

		Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_waveforms_update_timeout),
				WaveFormRefreshIntervalMs);

		std::unique_ptr<Recording> recording(new Recording(_audioProcessor.getCaptureChannels(), _recordingStartedFrame));
		for (auto i = 0U; i < _audioProcessor.getCaptureChannels(); ++i) {
			recording->waveForms[i] = manage(new WaveForm());
			//_channelsHBoxes[i]->pack_start(*recording->waveForms[i], Gtk::PACK_EXPAND_WIDGET);
			_channelsHBoxes[i]->pack_start(*recording->waveForms[i], Gtk::PACK_SHRINK);
		}
		show_all_children();

		_recordings.push_back(recording.get());
		recording.release();

		std::clog << "NOTICE: MainWindow::on_record_button_clicked(): Recording started" << std::endl;
		_recordButton.set_label("Stop recording");
	} else {
		_audioProcessor.stopRecord();
		_recordingFinishedFrame = _audioProcessor.getRecordFinishedFrame();
		std::clog << "NOTICE: MainWindow::on_record_button_clicked(): Recording stopped" << std::endl;
		_recordButton.set_label("Start recording");
	}
}

bool MainWindow::on_level_polling_timeout()
{
	std::size_t capturedFrames = _audioProcessor.getCapturedFrames();
	for (std::size_t i = 0U; i < _audioProcessor.getCaptureChannels(); ++i) {
		float level = _audioProcessor.getCaptureLevel(i, _volumeScannedFrame, capturedFrames);
		_levelIndicators[i]->set_fraction(level);
	}
	_volumeScannedFrame = capturedFrames;
	return true;
}

bool MainWindow::on_waveforms_update_timeout()
{
	std::size_t capturedFrames = _audioProcessor.getCapturedFrames();
	std::size_t newRecordingLevelExposedFrame = _isRecording ? capturedFrames : _recordingFinishedFrame;
	for (std::size_t i = 0U; i < _audioProcessor.getCaptureChannels(); ++i) {
		float level = _audioProcessor.getCaptureLevel(i, _recordingLevelExposedFrame, newRecordingLevelExposedFrame);
		_recordings.back()->waveForms[i]->addLevel(level);
	}
	_recordingLevelExposedFrame = newRecordingLevelExposedFrame;
	return _isRecording;
}
