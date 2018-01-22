#include "WaveForm.h"

#include <iostream>
#include <cassert>

namespace {

const Glib::ustring VerticalLineColorName("MidnightBlue");

}

WaveForm::WaveForm(bool isRecording) :
	Gtk::DrawingArea(),
	_isRecording(isRecording),
	_levels(),
	_vLineColor(VerticalLineColorName)
{
	Gdk::Colormap::get_system()->alloc_color(_vLineColor);
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

	// Drawing background
	auto bgColor = get_style()->get_dark(_isRecording ? Gtk::STATE_INSENSITIVE : Gtk::STATE_NORMAL);
	cr->set_source_rgb(bgColor.get_red_p(), bgColor.get_green_p(), bgColor.get_blue_p());
	cr->rectangle(event->area.x, event->area.y, event->area.width, event->area.height);
	cr->fill();

	// Drawing wave-form
	auto waveFormColor = get_style()->get_light(_isRecording ? Gtk::STATE_INSENSITIVE : Gtk::STATE_NORMAL);
	cr->set_source_rgb(waveFormColor.get_red_p(), waveFormColor.get_green_p(), waveFormColor.get_blue_p());
	cr->set_line_width(0.5);

	for (auto i = 0U; i < _levels.size(); ++i) {
		if (_isRecording && ((i + 1U) == _levels.size())) {
			break;
		}

		auto topY = static_cast<int>(
				(static_cast<double>(height) - static_cast<double>(height) * _levels[i]) / 2.0);
		auto bottomY = height - topY;
		cr->move_to(i, topY);
		cr->line_to(i, (topY == bottomY) ? bottomY + 1U : bottomY);

	}
	cr->stroke();

	// Drawing a current position vertical line
	if (_isRecording) {
		cr->set_source_rgb(_vLineColor.get_red_p(), _vLineColor.get_green_p(), _vLineColor.get_blue_p());
		cr->set_line_width(1.0);
		cr->move_to(width - 1, 0);
		cr->line_to(width - 1, height - 1);
		cr->stroke();
	}

	return true;
}
