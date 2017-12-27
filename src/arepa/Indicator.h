#ifndef __AREPA__INDICATOR_H
#define __AREPA__INDICATOR_H

//#include "CaptureChannel.h"

//#define ALSA_PCM_NEW_HW_PARAMS_API
//#include <alsa/asoundlib.h>

//#include <sndfile.hh>

#include <vector>
//#include <queue>
//#include <map>

#include <boost/atomic.hpp>
#include <boost/chrono.hpp>

//#include <ctime>

template <typename T> class Indicator
{
public:
	Indicator(std::size_t historySize);
	//~Indicator();

	void addMeasurement(const T& value);
	T getMax(std::size_t us, const T& defaultValue = T());
	T getMean(std::size_t us);
	T getMin(std::size_t us);
private:
	struct Measurement
	{
	//	Measurement() :
	//		ts()
		boost::chrono::steady_clock::time_point ts;
		T value;
	};

	typedef std::vector<Measurement> History;

	std::size_t _historySize;
	History _history;
	boost::atomic<std::size_t> _offset;
};

template <typename T> Indicator<T>::Indicator(std::size_t historySize) :
	_historySize(historySize),
	_history(historySize),
	_offset(0U)
{}

template <typename T> void Indicator<T>::addMeasurement(const T& value)
{
	std::size_t offset = _offset.fetch_add(1U) % _historySize;

	_history[offset].ts = boost::chrono::steady_clock::now();
	_history[offset].value = value;

	std::clog << "NOTICE: Indicator<T>::addMeasurement(" << value <<
		"): Added '" << value << "' value using " << offset << " offset" << std::endl;
}

template <typename T> T Indicator<T>::getMax(std::size_t us, const T& defaultValue)
{
	boost::chrono::steady_clock::time_point limit = boost::chrono::steady_clock::now() -
		boost::chrono::milliseconds(us);

	std::size_t offset = _offset.load() % _historySize;

	bool measurementFound = false;
	std::size_t i = 0;
	T maxValue(defaultValue);

	boost::chrono::steady_clock::time_point nullTs;
	while (true) {
		offset = (offset == 0U) ? _historySize - 1U : offset - 1U;

		if ((_history[offset].ts == nullTs) && (_history[offset].ts < limit)) {
			break;
		}

		std::clog << ">>>> offset: " << offset << ", _history[offset].ts: " << _history[offset].ts << ", limit: " << limit <<
			", _history[offset].value: " << _history[offset].value << std::endl;


		if (i > (_historySize / 2)) {
			std::cerr << "WARNING: Indicator<T>::getMax(" << us << ", " << defaultValue <<
				"): Indicator overflow detected, increase history size" << std::endl;
			break;
		}

		if (!measurementFound) {
			maxValue = _history[offset].value;
			measurementFound = true;
		} else if (_history[offset].value > maxValue) {
			maxValue = _history[offset].value;
		}

		++i;
	}

	if (!measurementFound) {
		std::cerr << "WARNING: Indicator<T>::getMax(" << us << ", " << defaultValue <<
			"): No measurement found, returning default value" << std::endl;
	}
	return maxValue;
}

template <typename T> T Indicator<T>::getMean(std::size_t us)
{
	return T();
}

template <typename T> T Indicator<T>::getMin(std::size_t us)
{
	return T();
}

#endif
