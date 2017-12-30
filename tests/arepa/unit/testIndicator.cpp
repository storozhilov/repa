#include "gtest/gtest.h"
#include "Indicator.h"

#include <limits>
#include <boost/thread.hpp>

extern "C" const int IndicatorMeasurements[] = { 10, 25, 15, 3, 18, 6, 21, 11 };

TEST(IndicatorTest, AddMeasurementsGetMax)
{
	Indicator<int> indicator(16U);
	EXPECT_EQ(indicator.getMax(1000U, std::numeric_limits<int>::min()), std::numeric_limits<int>::min());
	for (std::size_t i = 0U; i < (sizeof(IndicatorMeasurements) / sizeof(int)); ++i) {
		indicator.addMeasurement(IndicatorMeasurements[i]);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	}
	EXPECT_EQ(indicator.getMax(240U, std::numeric_limits<int>::min()), 21);
	EXPECT_EQ(indicator.getMax(440U, std::numeric_limits<int>::min()), 25);
}
