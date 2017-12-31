#include "WaveForm.h"

#include <iostream>

WaveForm::WaveForm() :
	Gtk::DrawingArea(),
	_levels()
{
#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	signal_expose_event().connect(sigc::mem_fun(*this, &WaveForm::on_expose_event), false);
#endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
}

void WaveForm::addLevel(float level)
{
	_levels.push_back(level);

	// TODO: Extend waveform
	set_size_request(_levels.size(), -1);
}

bool WaveForm::on_expose_event(GdkEventExpose * event)
{
	std::clog << "WaveForm::on_expose_event() event fired" << std::endl;
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (!window) {
		return true;
	}
	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
	const int height = allocation.get_height();

	std::clog << "WaveForm::on_expose_event() event fired in (" <<
		width << ", " << height << ") rectangle using event rectangle: (" <<
		event->area.width << ", " << event->area.height << ")" << std::endl;

	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
	cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
	cr->clip();

	//cr->set_line_width(10.0);
	//cr->set_source_rgb(0.8, 0.0, 0.0);
	cr->rectangle(0, 0, width, height);
	cr->stroke();
}
