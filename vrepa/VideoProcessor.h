#ifndef __VREPA__VIDEO_PROCESSOR_H
#define __VREPA__VIDEO_PROCESSOR_H

#include <glibmm/main.h>
#include <gstreamermm.h>

#include <map>
#include <mutex>

class VideoProcessor
{
public:
	typedef std::size_t SourceHandle;

	VideoProcessor();
	~VideoProcessor();

	void start();
	void stop();

	SourceHandle addSource(const char * url);
	void switchSource(const SourceHandle sourceHandle);
private:
	typedef std::map<SourceHandle, std::string> SourcesMap;

	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message);
	void on_rtspsrc_pad_added(const Glib::RefPtr<Gst::Pad>& newPad, Glib::RefPtr<Gst::Element> rtph264depay,
			SourceHandle sourceHandle);
	void on_selector_pad_added(const Glib::RefPtr<Gst::Pad>& newPad);

	Glib::RefPtr<Gst::Pipeline> _pipeline;
	Glib::RefPtr<Gst::Element> _inputSelector;
	Glib::RefPtr<Gst::Element> _mainSink;

	SourcesMap _sourcesMap;
	std::mutex _sourcesMapMutex;
};

#endif
