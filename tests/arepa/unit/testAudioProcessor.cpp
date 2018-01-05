#include "gtest/gtest.h"
#include "AudioProcessor.h"
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

#include <stdlib.h>
#include <boost/filesystem.hpp>

class AudioProcessorTest : public ::testing::Test
{
public:
	virtual void SetUp()
	{
		EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
		EXPECT_EQ(system("mkdir tmp_tests"), 0);
		//_ap.reset(new AudioProcessor("virtmic"));
		//_ap.reset(new AudioProcessor("default"));
		_ap.reset(new AudioProcessor("hw"));
		//boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}

	virtual void TearDown()
	{
		_ap.reset();
		//EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
	}

	std::unique_ptr<AudioProcessor> _ap;
};

TEST_F(AudioProcessorTest, RecordToInvalidLocation)
{
	ASSERT_THROW(_ap->startRecord("/foo/bar"), std::runtime_error);
}

TEST_F(AudioProcessorTest, RecordASecond)
{
	unsigned int captureChannels = _ap->getCaptureChannels();
	ASSERT_GT(captureChannels, 0U);

	time_t recordTs = _ap->startRecord("tmp_tests");

	std::size_t recordStartedPeriod = _ap->getRecordStartedPeriod();
	EXPECT_GT(recordStartedPeriod, 0U);
	std::size_t recordStartedFrame = _ap->getRecordStartedFrame();
	EXPECT_GT(recordStartedFrame, 0U);

	boost::this_thread::sleep_for(boost::chrono::milliseconds(1 * 1000));
	_ap->stopRecord();

	std::size_t recordFinishedPeriod = _ap->getRecordFinishedPeriod();
	EXPECT_GT(recordFinishedPeriod, recordStartedPeriod);
	std::size_t recordFinishedFrame = _ap->getRecordFinishedFrame();
	EXPECT_GT(recordFinishedFrame, recordStartedFrame);
	std::size_t capturedFrames = _ap->getCapturedFrames();
	EXPECT_GT(capturedFrames, recordFinishedFrame);

	std::clog << "NOTICE: TEST_F(AudioProcessorTest, RecordASecond): Recording started at " <<
		recordStartedFrame << "-th frame, recording finished at " << recordFinishedFrame <<
		"-th frame, " << capturedFrames << " frames captured in total" << std::endl;

	uintmax_t fileSize = 0U;
	for (unsigned int i = 0U; i < captureChannels; ++i) {
		std::ostringstream filename;
		filename << "tmp_tests/record_" << recordTs << ".track_" << std::setfill('0') << std::setw(2) << (i + 1) << ".wav";

		boost::filesystem::file_status s = boost::filesystem::status(filename.str());
		EXPECT_EQ(s.type(), boost::filesystem::regular_file);

		uintmax_t actualFileSize = boost::filesystem::file_size(filename.str());
		EXPECT_GT(actualFileSize, 0U);
		if (fileSize == 0U) {
			fileSize = actualFileSize;
		}
		EXPECT_EQ(actualFileSize, fileSize);

		// TODO: Check output files have a proper WAV-structure
	}
}
