#include "gtest/gtest.h"
#include "AudioProcessor.h"
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

class AudioProcessorTest : public ::testing::Test
{
//protected:
public:
	virtual void SetUp()
	{
		//_ap.reset(new AudioProcessor("virtmic"));
		//_ap.reset(new AudioProcessor("default"));
		_ap.reset(new AudioProcessor("hw"));
	}

	virtual void TearDown()
	{
		_ap.reset();
	}

	std::auto_ptr<AudioProcessor> _ap;
};

TEST_F(AudioProcessorTest, CreateCheckCapabilities)
{
	boost::this_thread::sleep_for(boost::chrono::milliseconds(10 * 1000));
	EXPECT_EQ(1, 1);
}
