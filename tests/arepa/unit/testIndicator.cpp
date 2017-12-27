#include "gtest/gtest.h"
#include "Indicator.h"

#include <limits>
#include <boost/thread.hpp>

extern "C" const int measurements[] = { 10, 25, 15, 3, 18, 6, 21, 11 };

TEST(IndicatorTest, CreateAddGet)
{
	boost::chrono::steady_clock::time_point ts;
	std::cout << ts << std::endl;
	std::cout << (sizeof(measurements) / sizeof(int)) << std::endl;

	Indicator<int> indicator(16U);
	EXPECT_EQ(indicator.getMax(1000U, std::numeric_limits<int>::min()), std::numeric_limits<int>::min());
	for (std::size_t i = 0U; i < (sizeof(measurements) / sizeof(int)); ++i) {
		indicator.addMeasurement(measurements[i]);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
	EXPECT_EQ(indicator.getMax(240, std::numeric_limits<int>::min()), 21);
	//EXPECT_EQ(indicator.getMax(440, std::numeric_limits<int>::min()), 25);

/*	Indicator<int> indicator2(16U);
	for (std::size_t i = 0U; i < (sizeof(measurements) / sizeof(int)); ++i) {
		indicator2.addMeasurement(measurements[i]);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
	EXPECT_EQ(indicator2.getMax(440, std::numeric_limits<int>::min()), 25);*/
}
