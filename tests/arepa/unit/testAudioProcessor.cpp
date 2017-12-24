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
	}

	virtual void TearDown()
	{
		_ap.reset();
		//EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
	}

	std::auto_ptr<AudioProcessor> _ap;
};

TEST_F(AudioProcessorTest, CreateCheckCapabilities)
{
	boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
	_ap->startRecord("tmp_tests");
	boost::this_thread::sleep_for(boost::chrono::milliseconds(4 * 1000));
	//_ap->stopRecord();
	EXPECT_EQ(1, 1);
}
