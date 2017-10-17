#include "VideoProcessor.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

VideoProcessor::VideoProcessor() :
	_pipeline(),
	_inputSelector()
{}

void VideoProcessor::start()
{
	_pipeline = Gst::Pipeline::create("video-processor-pipeline");

	_pipeline->get_bus()->add_watch(sigc::mem_fun(*this, &VideoProcessor::on_bus_message));

	Glib::RefPtr<Gst::Element> rtspsrc1 = Gst::ElementFactory::create_element("rtspsrc");
	if (!rtspsrc1) {
		throw std::runtime_error("Error creating 'rtspsrc' element");
	}
	rtspsrc1->set_property("location",
		Glib::ustring("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream"));
	Glib::RefPtr<Gst::Element> rtph264depay1 = Gst::ElementFactory::create_element("rtph264depay");
	if (!rtph264depay1) {
		throw std::runtime_error("Error creating 'rtph264depay' element");
	}
	Glib::RefPtr<Gst::Element> h264parse1 = Gst::ElementFactory::create_element("h264parse");
	if (!h264parse1) {
		throw std::runtime_error("Error creating 'h264parse' element");
	}

	Glib::RefPtr<Gst::Element> rtspsrc2 = Gst::ElementFactory::create_element("rtspsrc");
	if (!rtspsrc2) {
		throw std::runtime_error("Error creating 'rtspsrc' element");
	}
	rtspsrc2->set_property("location",
		Glib::ustring("rtsp://192.168.1.3:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream"));
	Glib::RefPtr<Gst::Element> rtph264depay2 = Gst::ElementFactory::create_element("rtph264depay");
	if (!rtph264depay2) {
		throw std::runtime_error("Error creating 'rtph264depay' element");
	}
	Glib::RefPtr<Gst::Element> h264parse2 = Gst::ElementFactory::create_element("h264parse");
	if (!h264parse2) {
		throw std::runtime_error("Error creating 'h264parse' element");
	}

	_inputSelector = Gst::ElementFactory::create_element("input-selector");
	if (!_inputSelector) {
		throw std::runtime_error("Error creating 'input-selector' element");
	}
	Glib::RefPtr<Gst::Element> vaapidecode = Gst::ElementFactory::create_element("vaapidecode");
	if (!vaapidecode) {
		throw std::runtime_error("Error creating 'vaapidecode' element");
	}
	Glib::RefPtr<Gst::Element> vaapisink = Gst::ElementFactory::create_element("vaapisink");
	if (!vaapisink) {
		throw std::runtime_error("Error creating 'vaapisink' element");
	}

	_pipeline->add(rtspsrc1)->add(rtph264depay1)->add(h264parse1);
	_pipeline->add(rtspsrc2)->add(rtph264depay2)->add(h264parse2);

	_pipeline->add(_inputSelector)->add(vaapidecode)->add(vaapisink);

	rtspsrc1->signal_pad_added().connect(sigc::bind(sigc::mem_fun(*this, &VideoProcessor::on_rtspsrc_pad_added), rtph264depay1));
	rtph264depay1->link(h264parse1)->link(_inputSelector);

	rtspsrc2->signal_pad_added().connect(sigc::bind(sigc::mem_fun(*this, &VideoProcessor::on_rtspsrc_pad_added), rtph264depay2));
	rtph264depay2->link(h264parse2)->link(_inputSelector);

	_inputSelector->link(vaapidecode)->link(vaapisink);

	_pipeline->set_state(Gst::STATE_PLAYING);

}

void VideoProcessor::stop()
{
	_pipeline->set_state(Gst::STATE_NULL);
	_pipeline.reset();
}

namespace {
	std::size_t sourceCounter = 0;
}

void VideoProcessor::switchSource(const SourceHandle&)
{
	std::size_t sourceIndex = sourceCounter++ % 2;
	std::cout << "sourceIndex: " << sourceIndex << std::endl;
	if (sourceIndex > 0) {
		_inputSelector->set_property("active-pad", _inputSelector->get_static_pad("sink_1"));
	} else {
		_inputSelector->set_property("active-pad", _inputSelector->get_static_pad("sink_0"));
	}
}

void VideoProcessor::on_rtspsrc_pad_added(const Glib::RefPtr<Gst::Pad>& newPad, Glib::RefPtr<Gst::Element> rtph264depay)
{
	Glib::RefPtr<Gst::Pad> sinkPad = rtph264depay->get_static_pad("sink");
	Gst::PadLinkReturn ret = newPad->link(sinkPad);

	if (ret != Gst::PAD_LINK_OK && ret != Gst::PAD_LINK_WAS_LINKED) {
		std::ostringstream msg;
		msg << "Linking of pads " << newPad->get_name() << " and " <<
			sinkPad->get_name() << " failed.";
		throw std::runtime_error(msg.str());
	}

	std::cout << "New pad added: " << newPad->get_name() << std::endl;
}

bool VideoProcessor::on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message)
{
	switch (message->get_message_type()) {
		case Gst::MESSAGE_EOS:
			std::cout << std::endl << "End of stream" << std::endl;
			return false;
		case Gst::MESSAGE_ERROR:
			std::cerr << "Error." << Glib::RefPtr<Gst::MessageError>::cast_static(message)->parse_debug() << std::endl;
			return false;
		default:
			break;
	}

	return true;
}
