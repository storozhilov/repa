#include "VideoProcessor.h"
#include <iostream>

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
	_pipeline = Gst::Pipeline::create("video-processor-pipeline");
	_pipeline->set_state(Gst::STATE_PLAYING);
	//_mainThread.reset(new std::thread([this] { this->process(); }));
}

void VideoProcessor::stop()
{
	//_mainThread->join();
	_pipeline->set_state(Gst::STATE_NULL);
	_pipeline.reset();
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
