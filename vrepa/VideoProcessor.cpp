#include "VideoProcessor.h"
#include "MainWindow.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <gstreamermm/videotestsrc.h>

VideoProcessor::VideoProcessor(MainWindow& mainWindow) :
	_mainWindow(mainWindow),
	_pipeline(),
	_inputSelector(),
	_mainSink(),
	_sourcesMap(),
	_sourcesMapMutex()
{
	_pipeline = Gst::Pipeline::create("video-processor-pipeline");

	_pipeline->get_bus()->enable_sync_message_emission();
	_pipeline->get_bus()->signal_sync_message().connect(sigc::mem_fun(*this, &VideoProcessor::on_bus_message_sync));
	_pipeline->get_bus()->add_watch(sigc::mem_fun(*this, &VideoProcessor::on_bus_message));

	_inputSelector = Gst::ElementFactory::create_element("input-selector");
	if (!_inputSelector) {
		throw std::runtime_error("Error creating 'input-selector' element");
	}
/*	_mainSink = Gst::ElementFactory::create_element("xvimagesink");
	if (!_mainSink) {
		throw std::runtime_error("Error creating 'xvimagesink' element");
	}*/
	_mainSink = Gst::ElementFactory::create_element("vaapisink");
	if (!_mainSink) {
		throw std::runtime_error("Error creating 'vaapisink' element");
	}

	_pipeline->add(_inputSelector)->add(_mainSink);
	_inputSelector->link(_mainSink);

	_inputSelector->signal_pad_added().connect(sigc::mem_fun(*this, &VideoProcessor::on_selector_pad_added));
}

void VideoProcessor::start()
{
	_pipeline->set_state(Gst::STATE_PLAYING);
}

VideoProcessor::~VideoProcessor()
{
	_pipeline.reset();
}

void VideoProcessor::stop()
{
	_pipeline->send_event(Gst::EventEos::create());
	// Await for file-sinks to recieve EOS?
	sleep(2);
	_pipeline->set_state(Gst::STATE_NULL);
}

VideoProcessor::SourceHandle VideoProcessor::addSource(const char * url)
{
	Glib::RefPtr<Gst::Element> tee = Gst::ElementFactory::create_element("tee");
	if (!tee) {
		throw std::runtime_error("Error creating 'tee' element");
	}
	Glib::RefPtr<Gst::Element> inputSelectorQueue = Gst::ElementFactory::create_element("queue");
	if (!inputSelectorQueue) {
		throw std::runtime_error("Error creating 'queue' element");
	}
	Glib::RefPtr<Gst::Element> sourceSinkQueue = Gst::ElementFactory::create_element("queue");
	if (!sourceSinkQueue) {
		throw std::runtime_error("Error creating 'queue' element");
	}
	Glib::RefPtr<Gst::Element> sourceSink = Gst::ElementFactory::create_element("xvimagesink");
	if (!sourceSink) {
		throw std::runtime_error("Error creating 'xvimagesink' element");
	}

	SourceHandle sourceHandle;
	{
		std::lock_guard<std::mutex> lock(_sourcesMapMutex);
		sourceHandle = _sourcesMap.empty() ? 1 : _sourcesMap.rbegin()->first + 1;
		SourceData sourceData;
		sourceData.sink = sourceSink;
		_sourcesMap.insert(SourcesMap::value_type(sourceHandle, sourceData));
	}

	Glib::RefPtr<Gst::Element> src;

	if (strcmp(url, SourceTestCircular) == 0) {
		src = Gst::ElementFactory::create_element("videotestsrc");
		if (!src) {
			throw std::runtime_error("Error creating 'videotestsrc' element");
		}
		src->set_property("pattern", Gst::VIDEO_TEST_SRC_CIRCULAR);
		_pipeline->add(src);
	} else if (strcmp(url, SourceTestSmpte100) == 0) {
		src = Gst::ElementFactory::create_element("videotestsrc");
		if (!src) {
			throw std::runtime_error("Error creating 'videotestsrc' element");
		}
		src->set_property("pattern", Gst::VIDEO_TEST_SRC_SMPTE100);
		_pipeline->add(src);
	} else {
		Glib::RefPtr<Gst::Element> rtspsrc = Gst::ElementFactory::create_element("rtspsrc");
		if (!rtspsrc) {
			throw std::runtime_error("Error creating 'rtspsrc' element");
		}
		rtspsrc->set_property("location", Glib::ustring(url));
		Glib::RefPtr<Gst::Element> rtph264depay = Gst::ElementFactory::create_element("rtph264depay");
		if (!rtph264depay) {
			throw std::runtime_error("Error creating 'rtph264depay' element");
		}
		Glib::RefPtr<Gst::Element> h264parse = Gst::ElementFactory::create_element("h264parse");
		if (!h264parse) {
			throw std::runtime_error("Error creating 'h264parse' element");
		}

		Glib::RefPtr<Gst::Element> inputTee = Gst::ElementFactory::create_element("tee");
		if (!inputTee) {
			throw std::runtime_error("Error creating 'tee' element");
		}
		Glib::RefPtr<Gst::Element> decoderQueue = Gst::ElementFactory::create_element("queue");
		if (!decoderQueue) {
			throw std::runtime_error("Error creating 'queue' element");
		}
		Glib::RefPtr<Gst::Element> fileQueue = Gst::ElementFactory::create_element("queue");
		if (!fileQueue) {
			throw std::runtime_error("Error creating 'queue' element");
		}
		Glib::RefPtr<Gst::Element> muxer = Gst::ElementFactory::create_element("mp4mux");
		if (!muxer) {
			throw std::runtime_error("Error creating 'mp4mux' element");
		}
		Glib::RefPtr<Gst::Element> fileSink = Gst::ElementFactory::create_element("filesink");
		if (!fileSink) {
			throw std::runtime_error("Error creating 'filesink' element");
		}
		std::ostringstream filename;
		filename << "video_cam_" << sourceHandle << ".mp4";
		fileSink->set_property("location", Glib::ustring(filename.str().c_str()));

		src = Gst::ElementFactory::create_element("vaapidecode");
		if (!src) {
			throw std::runtime_error("Error creating 'vaapidecode' element");
		}

		_pipeline->add(rtspsrc)->add(rtph264depay)->add(h264parse)->add(src);
		_pipeline->add(inputTee)->add(decoderQueue)->add(fileQueue)->add(muxer)->add(fileSink);

		rtspsrc->signal_pad_added().connect(sigc::bind(sigc::mem_fun(*this, &VideoProcessor::on_rtspsrc_pad_added),
				rtph264depay, sourceHandle));
		rtph264depay->link(h264parse)->link(inputTee)->link(decoderQueue)->link(src);
		inputTee->link(fileQueue)->link(muxer)->link(fileSink);
	}

	_pipeline->add(tee)->add(inputSelectorQueue)->add(sourceSinkQueue)->add(sourceSink);
	src->link(tee)->link(inputSelectorQueue)->link(_inputSelector);
	tee->link(sourceSinkQueue)->link(sourceSink);

	return sourceHandle;
}

void VideoProcessor::switchSource(const SourceHandle sourceHandle)
{
	bool sourceFound = false;
	std::string selectorPadName;
	{
		std::lock_guard<std::mutex> lock(_sourcesMapMutex);
		SourcesMap::const_iterator pos = _sourcesMap.find(sourceHandle);
		if (pos != _sourcesMap.end()) {
			sourceFound = true;
			selectorPadName = pos->second.inputSelectorPadName;
		}
	}

	if (!sourceFound) {
		std::cerr << "Source (" << sourceHandle << ") not found" << std::endl;
		return;
	}
	if (selectorPadName.empty()) {
		std::cerr << "Source (" << sourceHandle << ") does not have a pad on input selector" << std::endl;
		return;
	}

	std::cout << "Switching to " << sourceHandle << " source, input selector pad name: '" <<
		selectorPadName << "'" << std::endl;

	_inputSelector->set_property("active-pad", _inputSelector->get_static_pad(selectorPadName.c_str()));
}

void VideoProcessor::on_rtspsrc_pad_added(const Glib::RefPtr<Gst::Pad>& newPad, Glib::RefPtr<Gst::Element> rtph264depay,
		SourceHandle sourceHandle)
{
	Glib::RefPtr<Gst::Pad> sinkPad = rtph264depay->get_static_pad("sink");
	Gst::PadLinkReturn ret = newPad->link(sinkPad);

	if (ret != Gst::PAD_LINK_OK && ret != Gst::PAD_LINK_WAS_LINKED) {
		std::ostringstream msg;
		msg << "Linking of pads " << newPad->get_name() << " and " <<
			sinkPad->get_name() << " failed.";
		throw std::runtime_error(msg.str());
	}

	std::cout << "New source pad '" << newPad->get_name() << "' added to '" << newPad->get_parent_element()->get_name() <<
		"' element for " << sourceHandle << " source" << std::endl;
}

void VideoProcessor::on_selector_pad_added(const Glib::RefPtr<Gst::Pad>& newPad)
{
	SourceHandle sourceHandle;
	{
		std::lock_guard<std::mutex> lock(_sourcesMapMutex);
		SourcesMap::iterator pos = _sourcesMap.begin();
		while (pos != _sourcesMap.end()) {
			if (pos->second.inputSelectorPadName.empty()) {
				sourceHandle = pos->first;
				pos->second.inputSelectorPadName = newPad->get_name();
				break;
			}
			++pos;
		}
	}
	std::cout << "New sink pad added to '" << newPad->get_parent_element()->get_name() <<
		"' element for " << sourceHandle << " source: " << newPad->get_name() << std::endl;
}

void VideoProcessor::on_bus_message_sync(const Glib::RefPtr<Gst::Message>& message)
{
	if (!gst_is_video_overlay_prepare_window_handle_message (message->gobj())) {
		return;
	}

	if (_mainWindow._mainVideoAreaWindowHandle == 0) {
		std::cerr << "ERROR: Main video area window handle is not set" << std::endl;
		return;
	}

	GstVideoOverlay *overlay;
	overlay = GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (message->gobj()));

	Glib::RefPtr<Gst::Element> element = Glib::RefPtr<Gst::Element>::cast_dynamic(message->get_source());

	if (element == _mainSink) {
		gst_video_overlay_set_window_handle(overlay, _mainWindow._mainVideoAreaWindowHandle);
		std::cout << "VideoProcessor::on_bus_message_sync(): Video overlay of '" << element->get_name() <<
			"' element is attached to the main video area window handle" << std::endl;
	} else {
		SourceHandle sourceHandle;
		bool sourceFound = false;
		{
			std::lock_guard<std::mutex> lock(_sourcesMapMutex);
			SourcesMap::iterator pos = _sourcesMap.begin();
			while (pos != _sourcesMap.end()) {
				if (pos->second.sink == element) {
					sourceHandle = pos->first;
					sourceFound = true;
					break;
				}
				++pos;
			}
		}
		if (!sourceFound) {
			std::ostringstream msg;
			msg << "Source handle for '" << element->get_name() << "' element not found";
			throw std::runtime_error(msg.str());
		}

		// TODO: Thread-safety?
		guintptr videoAreaWindowHandle = _mainWindow._sourcesMap[sourceHandle].videoAreaWindowHandle;
		if (videoAreaWindowHandle == 0) {
			std::ostringstream msg;
			msg << "Video area for '" << element->get_name() << "' element is not realized yet";
			throw std::runtime_error(msg.str());
		}
		gst_video_overlay_set_window_handle(overlay, videoAreaWindowHandle);
		std::cout << "VideoProcessor::on_bus_message_sync(): Video overlay of '" << element->get_name() <<
			"' element is attached to the source video area window handle" << std::endl;
	}
}

bool VideoProcessor::on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message)
{
	switch (message->get_message_type()) {
		case Gst::MESSAGE_EOS:
			std::cout << std::endl << "End of stream" << std::endl;
			return false;
		case Gst::MESSAGE_ERROR:
			{
				Glib::RefPtr<Gst::Element> ms = Glib::RefPtr<Gst::Element>::cast_static(message->get_source());
				Glib::RefPtr<Gst::MessageError> me = Glib::RefPtr<Gst::MessageError>::cast_static(message);
				std::cerr << "VideoProcessor::on_bus_message(): Error message from '" <<
					ms->get_name() << "' element: '" << me->parse().what().c_str() <<
					"', debugging info: " << me->parse_debug() << std::endl;
				return false;
			}
		default:
			break;
	}

	return true;
}
