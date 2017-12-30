#ifndef __AREPA__WAVE_FORM_H
#define __AREPA__WAVE_FORM_H

#include <gtkmm.h>

class WaveForm : public Gtk::DrawingArea
{
public:
	WaveForm();

	void addLevel(std::size_t frameIndex, float level);
private:
	virtual bool on_expose_event(GdkEventExpose* event);
};

#endif
