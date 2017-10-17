#include "VideoProcessor.h"
#include <iostream>
#include <stdexcept>

VideoProcessor::VideoProcessor() :
	_mainThread(nullptr),
	_mainLoop(),
	_pipeline(),
	_source(),
	_decoder(),
	_playBin(),
	_rtph264depay()
{
	//_mainLoop = Glib::MainLoop::create();
	//_pipeline = Gst::Pipeline::create();
}

void VideoProcessor::start()
{
	//_mainLoop = Glib::MainLoop::create();

	_pipeline = Gst::Pipeline::create("video-processor-pipeline");

	_pipeline->get_bus()->add_watch(sigc::mem_fun(*this, &VideoProcessor::on_bus_message));

	Glib::RefPtr<Gst::Element> rtspsrc = Gst::ElementFactory::create_element("rtspsrc");
	if (!rtspsrc) {
		throw std::runtime_error("Error creating 'rtspsrc' element");
	}
	rtspsrc->set_property("location",
		Glib::ustring("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream"));
/*	Glib::RefPtr<Gst::Element> rtph264depay = Gst::ElementFactory::create_element("rtph264depay");
	if (!rtph264depay) {
		throw std::runtime_error("Error creating 'rtph264depay' element");
	}*/
	_rtph264depay = Gst::ElementFactory::create_element("rtph264depay");
	if (!_rtph264depay) {
		throw std::runtime_error("Error creating 'rtph264depay' element");
	}
	Glib::RefPtr<Gst::Element> h264parse = Gst::ElementFactory::create_element("h264parse");
	if (!h264parse) {
		throw std::runtime_error("Error creating 'h264parse' element");
	}
	Glib::RefPtr<Gst::Element> vaapidecode = Gst::ElementFactory::create_element("vaapidecode");
	if (!vaapidecode) {
		throw std::runtime_error("Error creating 'vaapidecode' element");
	}
	Glib::RefPtr<Gst::Element> vaapisink = Gst::ElementFactory::create_element("vaapisink");
	if (!vaapisink) {
		throw std::runtime_error("Error creating 'vaapisink' element");
	}

//	_pipeline->add(rtspsrc)->add(rtph264depay)->add(h264parse)->add(vaapidecode)->add(vaapisink);
	_pipeline->add(rtspsrc)->add(_rtph264depay)->add(h264parse)->add(vaapidecode)->add(vaapisink);

	//rtspsrc->link(rtph264depay)->link(h264parse)->link(vaapidecode)->link(vaapisink);
/*	if (!rtspsrc->link_pads("stream_0", rtph264depay, "sink")) {
		throw std::runtime_error("Error linking 'rtspsrc:stream_0' to 'rtph264depay:sink'");
	}*/
	rtspsrc->signal_pad_added().connect(sigc::mem_fun(*this, &VideoProcessor::on_rtspsrc_pad_added));
//	rtph264depay->link(h264parse)->link(vaapidecode)->link(vaapisink);
	_rtph264depay->link(h264parse)->link(vaapidecode)->link(vaapisink);

	//_pipeline->add(rtspsrc)->add(rtph264depay);
	//rtspsrc->link(rtph264depay);

	/*Glib::RefPtr<Gst::Element> videotestsrc = Gst::ElementFactory::create_element("videotestsrc");
	if (!videotestsrc) {
		throw std::runtime_error("Error creating 'videotestsrc' element");
	}
	Glib::RefPtr<Gst::Element> vaapisink = Gst::ElementFactory::create_element("vaapisink");
	if (!vaapisink) {
		throw std::runtime_error("Error creating 'vaapisink' element");
	}

	_pipeline->add(videotestsrc)->add(vaapisink);

	videotestsrc->link(vaapisink);*/

	_pipeline->set_state(Gst::STATE_PLAYING);

/*	if (!rtspsrc->link_pads("stream_0", rtph264depay, "sink")) {
		throw std::runtime_error("Error linking 'rtspsrc:stream_0' to 'rtph264depay:sink'");
	}*/

	//_mainThread.reset(new std::thread([this] { this->process(); }));
	//Glib::RefPtr<Gst::Element> playBin = Gst::ElementFactory::create_element("playbin");
	/*_playBin = Gst::ElementFactory::create_element("playbin");
	if (!_playBin) {
		throw std::runtime_error("Error creating 'playbin2' element");
	}
	_playBin->set_property("uri",
		Glib::ustring("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream"));
	_playBin->set_state(Gst::STATE_PLAYING);*/
}

void VideoProcessor::stop()
{
	//_playBin->set_state(Gst::STATE_NULL);
	//_playBin.reset();
	//_mainThread->join();

	_pipeline->set_state(Gst::STATE_NULL);
	_pipeline.reset();

	//_mainLoop.reset();
}

void VideoProcessor::process()
{
	_pipeline->set_state(Gst::STATE_PLAYING);
}

void VideoProcessor::on_rtspsrc_pad_added(const Glib::RefPtr<Gst::Pad>& newPad)
{
	//Glib::RefPtr<Gst::Pad> sinkPad = _pipeline.get_element("")->get_static_pad("sink");
	Glib::RefPtr<Gst::Pad> sinkPad = _rtph264depay->get_static_pad("sink");
	Gst::PadLinkReturn ret = newPad->link(sinkPad);

	if (ret != Gst::PAD_LINK_OK && ret != Gst::PAD_LINK_WAS_LINKED) {
		std::cerr << "Linking of pads " << newPad->get_name() << " and " <<
			sinkPad->get_name() << " failed." << std::endl;
	}

	std::cout << ">>>>>>>>>>>>>>>> New pad added: " << newPad->get_name() << std::endl;
}

bool VideoProcessor::on_bus_message(const Glib::RefPtr<Gst::Bus>&, const Glib::RefPtr<Gst::Message>& message)
{
	switch (message->get_message_type()) {
		case Gst::MESSAGE_EOS:
			std::cout << std::endl << "End of stream" << std::endl;
			//_mainLoop->quit();
			return false;
		case Gst::MESSAGE_ERROR:
			std::cerr << "Error." << Glib::RefPtr<Gst::MessageError>::cast_static(message)->parse_debug() << std::endl;
			//_mainLoop->quit();
			return false;
		default:
			break;
	}

	return true;
}
