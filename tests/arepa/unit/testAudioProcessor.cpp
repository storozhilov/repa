#include "gtest/gtest.h"
#include "AudioProcessor.h"

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
	EXPECT_EQ(1, 1);
}
