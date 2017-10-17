#ifndef __VREPA__VIDEO_PROCESSOR_H
#define __VREPA__VIDEO_PROCESSOR_H

#include <glibmm/main.h>
#include <gstreamermm.h>

class VideoProcessor
{
public:
	typedef std::size_t SourceHandle;

	VideoProcessor();

	void start();
	void stop();

	void switchSource(const SourceHandle&);
private:
	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message);
	void on_rtspsrc_pad_added(const Glib::RefPtr<Gst::Pad>& newPad, Glib::RefPtr<Gst::Element> rtph264depay);

	Glib::RefPtr<Gst::Pipeline> _pipeline;
	Glib::RefPtr<Gst::Element> _inputSelector;
};

#endif
