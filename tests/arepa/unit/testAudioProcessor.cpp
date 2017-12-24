#include "gtest/gtest.h"
#include "AudioProcessor.h"
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

#include <stdlib.h>

class AudioProcessorTest : public ::testing::Test
{
//protected:
public:
	virtual void SetUp()
	{
		EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
		EXPECT_EQ(system("mkdir tmp_tests"), 0);
		//_ap.reset(new AudioProcessor("virtmic"));
		//_ap.reset(new AudioProcessor("default"));
		_ap.reset(new AudioProcessor("hw"));
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}

	virtual void TearDown()
	{
		_ap.reset();
		//EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
	}

	std::auto_ptr<AudioProcessor> _ap;
};

TEST_F(AudioProcessorTest, RecordToInvalidLocation)
{
	ASSERT_THROW(_ap->startRecord("/foo/bar"), std::runtime_error);
}

TEST_F(AudioProcessorTest, RecordASecond)
{
	time_t recordTs = _ap->startRecord("tmp_tests");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(1 * 1000));
	_ap->stopRecord();
	// TODO: Check output files exist and have a proper structure
}
