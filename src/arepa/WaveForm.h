#ifndef __AREPA__WAVE_FORM_H
#define __AREPA__WAVE_FORM_H

#include <gtkmm.h>

class WaveForm : public Gtk::DrawingArea
{
public:
	WaveForm();

	void addLevel(float level);
private:
	typedef std::deque<float> LevelsContainer;

	virtual bool on_expose_event(GdkEventExpose* event);

	LevelsContainer _levels;
};

#endif
