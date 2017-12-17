#ifndef __VREPA__VIDEO_PROCESSOR_H
#define __VREPA__VIDEO_PROCESSOR_H

#include <glibmm/main.h>
#include <gstreamermm.h>

#include <map>
#include <mutex>

class MainWindow;

class VideoProcessor
{
public:
	static constexpr const char * SourceTestCircular = "videotestsrc:circular";
	static constexpr const char * SourceTestSmpte100 = "videotestsrc:smpte";
	
	typedef std::size_t SourceHandle;

	VideoProcessor(MainWindow& mainWindow);
	~VideoProcessor();

	void start();
	void stop();

	SourceHandle addSource(const char * url);
	void switchSource(const SourceHandle sourceHandle);
private:
	struct SourceData {
		std::string inputSelectorPadName;
		Glib::RefPtr<Gst::Element> sink;
	};
	typedef std::map<SourceHandle, SourceData> SourcesMap;

	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message);
	void on_bus_message_sync(const Glib::RefPtr<Gst::Message>& message);
	void on_rtspsrc_pad_added(const Glib::RefPtr<Gst::Pad>& newPad, Glib::RefPtr<Gst::Element> rtph264depay,
			SourceHandle sourceHandle);
	void on_selector_pad_added(const Glib::RefPtr<Gst::Pad>& newPad);

	MainWindow& _mainWindow;
	Glib::RefPtr<Gst::Pipeline> _pipeline;
	Glib::RefPtr<Gst::Element> _inputSelector;
	Glib::RefPtr<Gst::Element> _mainSink;

	SourcesMap _sourcesMap;
	std::mutex _sourcesMapMutex;
};

#endif
