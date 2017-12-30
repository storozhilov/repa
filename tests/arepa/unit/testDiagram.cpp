#include "gtest/gtest.h"
#include "Diagram.h"

#include <limits>

extern "C" const int DiagramMeasurements[] = { 10, 25, 15, 3, 18, 6, 21, 11 };

TEST(DiagramTest, AddMeasurementsGetMax)
{
	Diagram<int> diagram(32U);
	for (std::size_t i = 0U; i < (sizeof(DiagramMeasurements) / sizeof(int)); ++i) {
		std::size_t index = diagram.addMeasurement(DiagramMeasurements[i]);
		EXPECT_EQ(index, i);
	}

	int maxValue = std::numeric_limits<int>::min();
	std::size_t newIndex = diagram.forEach(0U, [&maxValue](std::size_t index, int value) {
		if (value > maxValue) {
			maxValue = value;
		}
	});
	EXPECT_EQ(newIndex, 7U);
	EXPECT_EQ(maxValue, 25);

	int minValue = std::numeric_limits<int>::max();
	newIndex = diagram.forEach(4U, [&minValue](std::size_t index, int value) {
		if (value < minValue) {
			minValue = value;
		}
	});
	EXPECT_EQ(newIndex, 7U);
	EXPECT_EQ(minValue, 6);

	Diagram<int> diagram1(32U);
	for (std::size_t i = 0U; i < 1024; ++i) {
		std::size_t index = diagram1.addMeasurement(i);
		EXPECT_EQ(index, i);
	}

	minValue = std::numeric_limits<int>::max();
	newIndex = diagram1.forEach(1024U - 16U - 1U, [&minValue](std::size_t index, int value) {
		if (value < minValue) {
			minValue = value;
		}
	});
	EXPECT_EQ(newIndex, 1024U - 1U);
	EXPECT_EQ(minValue, 1024U - 16U - 1U);
}
