#include "WaveForm.h"

#include <iostream>
#include <cassert>

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
	set_size_request(_levels.size(), -1);
}

bool WaveForm::on_expose_event(GdkEventExpose * event)
{
	Glib::RefPtr<Gdk::Window> window = get_window();
	if (!window) {
		return true;
	}

	Gtk::Allocation allocation = get_allocation();
	auto width = allocation.get_width();
	auto height = allocation.get_height();
	assert(_levels.size() == 0U || width == _levels.size());

	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
	cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
	cr->clip();

	auto waveFormColor = get_style()->get_dark(Gtk::STATE_INSENSITIVE);
	cr->set_source_rgb(
			static_cast<float>(waveFormColor.get_red()) / static_cast<float>(std::numeric_limits<gushort>::max()),
			static_cast<float>(waveFormColor.get_green()) / static_cast<float>(std::numeric_limits<gushort>::max()),
			static_cast<float>(waveFormColor.get_blue()) / static_cast<float>(std::numeric_limits<gushort>::max()));

	for (auto i = 0U; i < _levels.size(); ++i) {
		auto topY = static_cast<int>(
				(static_cast<float>(height) - static_cast<float>(height) * _levels[i]) / 2.0F);
		auto bottomY = height - topY;
		cr->move_to(i, topY);
		cr->line_to(i, (topY == bottomY) ? bottomY + 1U : bottomY);
		cr->stroke_preserve();
	}
	return true;
}
