#include "MainWindow.h"
#include "WaveForm.h"

MainWindow::MainWindow(AudioProcessor& audioProcessor, const Glib::ustring& outputPath) :
	Gtk::Window(),
	_audioProcessor(audioProcessor),
	_outputPath(outputPath),
	_isRecording(false),
	_vbox(false, 6),
	_buttonBox(Gtk::BUTTONBOX_START, 6),
	_recordButton("Start recording"),
	_levelIndicators(_audioProcessor.getCaptureChannels()),
	_waveForms(_audioProcessor.getCaptureChannels())
{
	add(_vbox);
	_vbox.pack_start(_buttonBox, Gtk::PACK_SHRINK);
	_buttonBox.pack_start(_recordButton, Gtk::PACK_SHRINK);
	_recordButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_record_button_clicked));

	for (std::size_t i = 0U; i < _audioProcessor.getCaptureChannels(); ++i) {
		Gtk::HBox * hbox = manage(new Gtk::HBox(false, 6));
		_vbox.pack_start(*hbox, Gtk::PACK_SHRINK);

		_levelIndicators[i] = manage(new Gtk::ProgressBar());
		_levelIndicators[i]->set_orientation(Gtk::PROGRESS_BOTTOM_TO_TOP);
		//_levelIndicators[i]->set_fraction(0.25);
		hbox->pack_start(*_levelIndicators[i], Gtk::PACK_SHRINK);

		_waveForms[i] = manage(new WaveForm());
		hbox->pack_start(*_waveForms[i], Gtk::PACK_EXPAND_WIDGET);
	}
	show_all_children();

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_level_polling_timeout),
			LevelRefreshIntervalMs);
}

void MainWindow::on_record_button_clicked()
{
	_isRecording = !_isRecording;
	if (_isRecording) {
		_audioProcessor.startRecord(_outputPath);
		std::clog << "NOTICE: MainWindow::on_record_button_clicked(): Recording started" << std::endl;
		_recordButton.set_label("Stop recording");
	} else {
		_audioProcessor.stopRecord();
		std::clog << "NOTICE: MainWindow::on_record_button_clicked(): Recording stopped" << std::endl;
		_recordButton.set_label("Start recording");
	}
}

bool MainWindow::on_level_polling_timeout()
{
	for (std::size_t i = 0U; i < _audioProcessor.getCaptureChannels(); ++i) {
		float level = _audioProcessor.getCaptureLevel(i);
		_levelIndicators[i]->set_fraction(level);
	}

	return true;
}
