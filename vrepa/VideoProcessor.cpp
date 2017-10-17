#include "VideoProcessor.h"
#include <iostream>
#include <stdexcept>

VideoProcessor::VideoProcessor() :
	_mainThread(nullptr),
	_mainLoop(),
	_pipeline(),
	_source(),
	_decoder(),
	_playBin()
{
	//_mainLoop = Glib::MainLoop::create();
	//_pipeline = Gst::Pipeline::create();
}

void VideoProcessor::start()
{
	//_mainLoop = Glib::MainLoop::create();
	//_pipeline = Gst::Pipeline::create("video-processor-pipeline");
	//_pipeline->set_state(Gst::STATE_PLAYING);
	//_mainThread.reset(new std::thread([this] { this->process(); }));
	//Glib::RefPtr<Gst::Element> playBin = Gst::ElementFactory::create_element("playbin");
	_playBin = Gst::ElementFactory::create_element("playbin");
	if (!_playBin) {
		throw std::runtime_error("Error creating 'playbin2' element");
	}
	_playBin->set_property("uri",
		Glib::ustring("rtsp://192.168.1.2:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream"));
	_playBin->set_state(Gst::STATE_PLAYING);
}

void VideoProcessor::stop()
{
	_playBin->set_state(Gst::STATE_NULL);
	_playBin.reset();
	//_mainThread->join();
	//_pipeline->set_state(Gst::STATE_NULL);
	//_pipeline.reset();
	//_mainLoop.reset();
}

void VideoProcessor::process()
{
	_pipeline->set_state(Gst::STATE_PLAYING);
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
