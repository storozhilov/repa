#ifndef __VREPA__VIDEO_PROCESSOR_H
#define __VREPA__VIDEO_PROCESSOR_H

#include <glibmm/main.h>
#include <gstreamermm.h>
#include <memory>
#include <thread>

class VideoProcessor
{
public:
	VideoProcessor();

	void start();
	void stop();
private:
	void process();
	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message);

	std::unique_ptr<std::thread> _mainThread;
	Glib::RefPtr<Glib::MainLoop> _mainLoop;
	Glib::RefPtr<Gst::Pipeline> _pipeline;
	Glib::RefPtr<Gst::Element> _source;
	Glib::RefPtr<Gst::Element> _decoder;
};

#endif
