#include <iostream>
#include "MultitrackRecorder.h"

int main(int argc, char * argv[]) {
	std::cout << "Starting multitrack recorder" << std::endl;
	MultitrackRecorder recorder;
	recorder.start();
	// TODO: Await termination signal
	recorder.stop();
	return 0;
}
