#include "gtest/gtest.h"
#include "CaptureChannel.h"
#include <memory>

class CaptureChannelTest : public ::testing::Test
{
//protected:
public:
	virtual void SetUp()
	{
		EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
		EXPECT_EQ(system("mkdir tmp_tests"), 0);
	}

	virtual void TearDown()
	{
		//EXPECT_EQ(system("rm -Rf tmp_tests"), 0);
	}
};

TEST_F(CaptureChannelTest, CreateAndCloseFile)
{
	std::auto_ptr<CaptureChannel> cc(new CaptureChannel(44100, SND_PCM_FORMAT_S16_LE));
	cc->openFile("tmp_tests/capture_channel_test_file.wav");
	cc->closeFile();
	EXPECT_EQ(system("stat tmp_tests/capture_channel_test_file.wav"), 0);
}
