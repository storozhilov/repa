#include "gtest/gtest.h"
#include "Diagram.h"

#include <limits>

extern "C" const int DiagramMeasurements[] = { 10, 25, 15, 3, 18, 6, 21, 11 };

#define INDEX_OFFSET 64

TEST(RingDiagramTest, AddMeasurementsGetMax)
{
	Diagram<std::size_t, int> diagram(32U);
	EXPECT_EQ(diagram.getIndex(), 0U);

	int maxValue = std::numeric_limits<int>::min();
	std::size_t callbackCallsCount = 0U;
	diagram.forEach(0U, 1U, [&maxValue, &callbackCallsCount](std::size_t index, int value) {
		++callbackCallsCount;
		if (value > maxValue) {
			maxValue = value;
		}
	});
	EXPECT_EQ(maxValue, std::numeric_limits<int>::min());
	EXPECT_EQ(callbackCallsCount, 0U);

	for (std::size_t i = 0U; i < (sizeof(DiagramMeasurements) / sizeof(int)); ++i) {
		diagram.addMeasurement(i + 1, DiagramMeasurements[i]);
	}
	EXPECT_EQ(diagram.getIndex(), sizeof(DiagramMeasurements) / sizeof(int));

	for (std::size_t i = 0U; i < (sizeof(DiagramMeasurements) / sizeof(int)); ++i) {
		diagram.addMeasurement(i + INDEX_OFFSET + 1, DiagramMeasurements[i]);
	}
	EXPECT_EQ(diagram.getIndex(), sizeof(DiagramMeasurements) / sizeof(int) + INDEX_OFFSET);

	diagram.forEach(32U, 48U, [&maxValue, &callbackCallsCount](std::size_t index, int value) {
		++callbackCallsCount;
		if (value > maxValue) {
			maxValue = value;
		}
	});
	EXPECT_EQ(maxValue, std::numeric_limits<int>::min());
	EXPECT_EQ(callbackCallsCount, 0U);

	diagram.forEach(0U, INDEX_OFFSET + sizeof(DiagramMeasurements) / sizeof(int),
		[&maxValue, &callbackCallsCount](std::size_t index, int value) {
			++callbackCallsCount;
			if (value > maxValue) {
				maxValue = value;
			}
		}
	);
	EXPECT_EQ(maxValue, 25);
	EXPECT_EQ(callbackCallsCount, 16U);

	maxValue = std::numeric_limits<int>::min();
	callbackCallsCount = 0U;
	diagram.forEach(4U, sizeof(DiagramMeasurements) / sizeof(int),
		[&maxValue, &callbackCallsCount](std::size_t index, int value) {
			++callbackCallsCount;
			if (value > maxValue) {
				maxValue = value;
			}
		}
	);
	EXPECT_EQ(maxValue, 21);
	EXPECT_EQ(callbackCallsCount, 4U);
}
