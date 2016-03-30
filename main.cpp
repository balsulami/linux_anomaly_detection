
#define DEBUG_LOG 1
#ifdef DEBUG_LOG
#include<fstream>
std::ofstream traces_out("/root/.clion11/system/cmake/generated/e3f8c42a/e3f8c42a/Release/traces.txt");
std::ofstream debug_out("/root/.clion11/system/cmake/generated/e3f8c42a/e3f8c42a/Release/debug.txt");
#endif

#ifdef FRONT_PROCESS
#define output_stream std::cout
#else
std::ofstream _output("/root/.clion11/system/cmake/generated/e3f8c42a/e3f8c42a/Release/output.txt");
#define output_stream _output
#endif


#include <iostream>
#include <fstream>
#include"sensor/multithreaded_sensor.h"
#include "utils/sys_record.h"
#include "detector/anomaly_detector.h"

typedef SysRecord sys_record;

int main(int argc, char ** argv) {
	std::string sensor_name = "syscall_detector";
	std::string run_path = "/var/run";

	std::vector<std::string> args;
	std::transform(argv + 1, argv + argc, std::back_inserter(args), [](char *arg) { return std::string(arg); });

	// Reteriving the pid of the last running detector
	int pid = get_sensor_pid(sensor_name, run_path);
	// check if the sensor is already running
	if (kill(pid, 0) == ESRCH)
		// the pid exists but the process is not running
		// remove the pid file
		remove_sensor_pid(sensor_name, run_path);

	std::vector<std::string>::iterator iter;
	if ((iter = std::find(args.begin(), args.end(), "stop")) != args.end()) {
		if (pid != -1) {
			// pass the command stop
			std::cout << "Stopping the daemon process pid= " << pid << "\n";
			AnomalyDetector<sys_record> detector;
			kill_sensor(pid);
			detector.stop();
			remove_sensor_pid(sensor_name, run_path);
			return 0;
		} else {
			std::cout << "The sensor is NOT running.\n";
			return 0;
		}
	}
	else if ((iter = std::find(args.begin(), args.end(), "start")) != args.end()) {
		if (pid ==-1) {
			setup_daemon(sensor_name, run_path);
			AnomalyDetector<sys_record> detector(2,2);
			detector.start();
		}
		else{
			std::cout << "The sensor is already running.\n";
			return 0;
		}
	}
}
