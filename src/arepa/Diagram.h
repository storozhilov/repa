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

//------------------------------------------------------------------------------

template <typename I, typename V> class RingDiagram
{
public:
	RingDiagram(std::size_t historySize);
	//~RingDiagram();

	I getIndex() const;
	void addMeasurement(const I& index, const V& value);
	void forEach(const I& begin, const I& end, std::function<void(I, V)>) const;
private:
	typedef std::pair<I, V> Measurement;
	typedef std::vector<Measurement> History;

	std::size_t _historySize;
	History _history;
	std::atomic<std::size_t> _historyIndex;
};

template <typename I, typename V> RingDiagram<I, V>::RingDiagram(std::size_t historySize) :
	_historySize(historySize),
	_history(historySize),
	_historyIndex(0U)
{}

template <typename I, typename V> I RingDiagram<I, V>::getIndex() const
{
	std::size_t historyIndex = _historyIndex.load();
	if (historyIndex == 0U) {
		std::cerr << "WARNING: RingDiagram<I, V>::getIndex(): " <<
			"No measurements found" << std::endl;
		return I();
	}
	std::size_t offset = (historyIndex - 1) % _historySize;
	return _history[offset].first;
}

template <typename I, typename V> void RingDiagram<I, V>::addMeasurement(const I& index, const V& value)
{
	std::size_t historyIndex = _historyIndex.fetch_add(1U);
	std::size_t offset = historyIndex % _historySize;
	_history[offset].first = index;
	_history[offset].second = value;
}

template <typename I, typename V> void RingDiagram<I, V>::forEach(const I& begin,
		const I& end, std::function<void(I, V)> func) const
{
	std::size_t initialHistoryIndex = _historyIndex.load();
	if (initialHistoryIndex == 0U) {
		std::cerr << "WARNING: RingDiagram<I, V>::forEach(" << begin << ", " <<
			end << "): " << "No measurements found" << std::endl;
		return;
	}
	--initialHistoryIndex;
	std::size_t historyIndex = initialHistoryIndex;

	// Passing by items after the 'end'
	while (_history[historyIndex % _historySize].first > end) {
		if (historyIndex == 0U) {
			std::cerr << "WARNING: RingDiagram<I, V>::forEach(" << begin << ", " << end << "): "
				<< "No matching measurements found" << std::endl;
			return;
		}
		--historyIndex;
		if ((initialHistoryIndex - historyIndex) > (_historySize / 2)) {
			std::cerr << "WARNING: RingDiagram<I, V>::forEach(" << begin << ", " << end << "): "
				<< "Diagram history over-run detected, try to increase history size" << std::endl;
			return;
		}
	}

	// Calling user function for matching items
	bool measurementsFound = false;
	while (_history[historyIndex % _historySize].first >= begin) {
		measurementsFound = true;
		func(_history[historyIndex % _historySize].first, _history[historyIndex % _historySize].second);

		if (historyIndex == 0U) {
			break;;
		}
		--historyIndex;
		if ((initialHistoryIndex - historyIndex) > (_historySize / 2)) {
			std::cerr << "WARNING: RingDiagram<I, V>::forEach(" << begin << ", " << end << "): "
				<< "Diagram history over-run detected, try to increase history size" << std::endl;
			break;
		}
	}
	if (!measurementsFound) {
		std::cerr << "WARNING: RingDiagram<I, V>::forEach(" << begin << ", " << end << "): "
			<< "No matching measurements found" << std::endl;
	}
}

#endif
