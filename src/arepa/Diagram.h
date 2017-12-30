#ifndef __AREPA__DIAGRAM_H
#define __AREPA__DIAGRAM_H

#include <atomic>
#include <functional>

template <typename I, typename V> class Diagram
{
public:
	Diagram(std::size_t historySize);
	//~Diagram();

	I getIndex() const;
	void addMeasurement(const I& index, const V& value);
	void forEach(const I& after, const I& end, std::function<void(I, V)>) const;
private:
	typedef std::pair<I, V> Measurement;
	typedef std::vector<Measurement> History;

	std::size_t _historySize;
	History _history;
	std::atomic<std::size_t> _historyIndex;
};

template <typename I, typename V> Diagram<I, V>::Diagram(std::size_t historySize) :
	_historySize(historySize),
	_history(historySize),
	_historyIndex(0U)
{}

template <typename I, typename V> I Diagram<I, V>::getIndex() const
{
	std::size_t historyIndex = _historyIndex.load();
	if (historyIndex == 0U) {
		std::cerr << "WARNING: Diagram<I, V>::getIndex(): " <<
			"No measurements found" << std::endl;
		return I();
	}
	std::size_t offset = (historyIndex - 1) % _historySize;
	return _history[offset].first;
}

template <typename I, typename V> void Diagram<I, V>::addMeasurement(const I& index, const V& value)
{
	std::size_t historyIndex = _historyIndex.fetch_add(1U);
	std::size_t offset = historyIndex % _historySize;
	_history[offset].first = index;
	_history[offset].second = value;
}

template <typename I, typename V> void Diagram<I, V>::forEach(const I& after,
		const I& end, std::function<void(I, V)> func) const
{
	std::size_t initialHistoryIndex = _historyIndex.load();
	if (initialHistoryIndex == 0U) {
		std::cerr << "WARNING: Diagram<I, V>::forEach(" << after << ", " <<
			end << "): " << "No measurements found" << std::endl;
		return;
	}
	--initialHistoryIndex;
	std::size_t historyIndex = initialHistoryIndex;

	// Passing by items after the 'end'
	while (_history[historyIndex % _historySize].first > end) {
		if (historyIndex == 0U) {
			std::cerr << "WARNING: Diagram<I, V>::forEach(" << after << ", " << end << "): "
				<< "No matching measurements found" << std::endl;
			return;
		}
		--historyIndex;
		if ((initialHistoryIndex - historyIndex) > (_historySize / 2)) {
			std::cerr << "WARNING: Diagram<I, V>::forEach(" << after << ", " << end << "): "
				<< "Diagram history over-run detected, try to increase history size" << std::endl;
			return;
		}
	}

	// Calling user function for matching items
	bool measurementsFound = false;
	while (_history[historyIndex % _historySize].first > after) {
		measurementsFound = true;
		func(_history[historyIndex % _historySize].first, _history[historyIndex % _historySize].second);

		if (historyIndex == 0U) {
			break;;
		}
		--historyIndex;
		if ((initialHistoryIndex - historyIndex) > (_historySize / 2)) {
			std::cerr << "WARNING: Diagram<I, V>::forEach(" << after << ", " << end << "): "
				<< "Diagram history over-run detected, try to increase history size" << std::endl;
			break;
		}
	}
	if (!measurementsFound) {
		std::cerr << "WARNING: Diagram<I, V>::forEach(" << after << ", " << end << "): "
			<< "No matching measurements found" << std::endl;
	}
}

#endif
