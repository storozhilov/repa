#ifndef __AREPA__DIAGRAM_H
#define __AREPA__DIAGRAM_H

#include <atomic>
#include <functional>

template <typename T> class Diagram
{
public:
	typedef std::vector<T> History;

	Diagram(std::size_t historySize);
	//~Diagram();

	std::size_t getIndex() const;
	std::size_t addMeasurement(const T& value);
	std::size_t forEach(std::size_t startingFrom, std::function<void(std::size_t, T)>) const;
private:
	std::size_t _historySize;
	History _history;
	std::atomic<std::size_t> _index;
};

template <typename T> Diagram<T>::Diagram(std::size_t historySize) :
	_historySize(historySize),
	_history(historySize),
	_index(0U)
{}

template <typename T> std::size_t Diagram<T>::getIndex() const
{
	std::size_t index = _index.load();
	return (index > 0U) ? index - 1 : 0U;
}

template <typename T> std::size_t Diagram<T>::addMeasurement(const T& value)
{
	std::size_t index = _index.fetch_add(1U);
	std::size_t offset = index % _historySize;
	_history[offset] = value;
	return index;
}

template <typename T> std::size_t Diagram<T>::forEach(std::size_t startingFrom,
		std::function<void(std::size_t, T)> func) const
{
	std::size_t index = _index.load();
	if (index == 0U) {
		// Diagram is empty
		return 0U;
	}
	--index;

	for (std::size_t i = index; i >= startingFrom; --i) {
		if ((index - i) > (_historySize / 2)) {
			std::cerr << "WARNING: Diagram<T>::forEach(" << startingFrom << ", <func>): " <<
				"Diagram history over-run detected, increase history size" << std::endl;
			break;
		}

		std::size_t offset = i % _historySize;
		func(i, _history[offset]);

		if (i == 0U) {
			break;
		}
	}
	return index;
}

#endif
