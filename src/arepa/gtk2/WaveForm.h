#ifndef __AREPA__WAVE_FORM_H
#define __AREPA__WAVE_FORM_H

#include <gtkmm.h>

class WaveForm : public Gtk::DrawingArea
{
public:
	WaveForm(bool isRecording);

	void addLevel(float level);
	inline void setIsRecording(bool isRecording) {
		_isRecording = isRecording;
	}
private:
	typedef std::deque<float> LevelsContainer;

	virtual bool on_expose_event(GdkEventExpose* event);

	bool _isRecording;
	LevelsContainer _levels;
	Gdk::Color _vLineColor;
};

#endif
