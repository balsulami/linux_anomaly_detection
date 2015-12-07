

#include "utils/types.h"
#include <iostream>
#include <fstream>
#include"sensor/single_sensor.h"
#include"sensor/multithreaded_sensor.h"
#include"stream/console_stream.h"
#include "stream/dumy_stream.h"
#include "stream/socket_stream.h"
#include "utils/sys_record.h"
#include "detector/anomaly_detector.h"

std::string current_dir;

typedef SysRecord sys_record;

/*
#  -p  pid or program name
#  -pp program name (find all )
#  -h host:port
#  -m c,n,v ;// c compact,n names, v verbose
#  -o output file
#  -t time, in seconds, to send write data out
#  -s buffer size
#  -d 1000
#   Example
#   -p=1234 -s=0
#   Results
#   [process_name,pid,record time, syscall name]
*/
void usage(int argc, char *argv[]) {
    if (argc >=1) {
        printf("Usage: %s -h -nn", argv[0]);
        printf("  Options:n");
        printf("      -ntDon't fork off as a daemon.n");
        printf("      -htShow this help screen.n");
        printf("n");
    }
}

#include <sys/stat.h>


void setup_daemon(const std::string & deamon_name,const std::string & run_path) {
    //------------
    //Daemon Setup
    //------------
    pid_t pid, sid;


    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
           we can exit the parent process. */
    if (pid > 0) {
        std::cout << "Creating a daemon process pid= " << pid << "\n";
        std::cout << "You can stop the process by the following command '" << deamon_name << " stop\n";
        exit(EXIT_SUCCESS);
    }

    write_to(run_path + "/" + deamon_name + ".pid", std::to_string(getpid()));

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}


int get_sensor_pid(const std::string & deamon_name, const std::string & run_path){
	std::ifstream pid_file(run_path + "/" + deamon_name + ".pid");
	int pid = -1;
	if (pid_file.good())
		pid_file >> pid;
	pid_file.close();
	return pid;
}

int remove_sensor_pid(const std::string & deamon_name, const std::string & run_path){
	std::remove((run_path + "/" + deamon_name + ".pid").c_str());
}

int kill_sensor(int pid){
	write_to(FTRACE_PATH + "/tracing_on", "0");
	write_to(SYSCALLS_PATH + "/filter", "0");
	write_to(SYSCALLS_PATH + "/" + SYS_ENTRY + "/" + "filter", "0");
	write_to(SYSCALLS_PATH + "/" + SYS_EXIT + "/filter", "0");
	sleep_sec(5);
	auto curr_pid = get_pid();
	auto pids = get_pids(pid);
	for (auto itr = pids.begin();itr < pids.end(); itr++){
		if(curr_pid != *itr)
			kill(*itr, SIGKILL);
	}
	kill(pid, SIGKILL);
}

template<typename T>
sensor_stream<T> * setup_stream(std::vector<std::string> & args){
    auto iter = std::find(args.begin(), args.end(), "-h");
	if (iter != args.end()){
		std::string value = *(iter + 1);
		auto split = value.find(':');

		std::string hostname = value.substr(0, split);
		std::string port = value.substr(split + 1);
		return new socket_stream<T>("sensor1", hostname.c_str(), std::atoi(port.c_str()));
	}
    iter = std::find(args.begin(), args.end(), "-o");
    if (iter != args.end()){
        return new file_aggr_stream<T>((iter + 1)->c_str());
    }
    iter = std::find(args.begin(), args.end(), "-c");
    if (iter != args.end()){
        return new console_stream<T>();
    }
    return new dumy_stream<T>();
}

int main(int argc, char ** argv) {
	std::string sensor_name = "syscall_sensor";
	std::string run_path = "/var/run";

	std::vector<std::string> args;
	std::transform(argv + 1, argv + argc, std::back_inserter(args), [](char *arg) { return std::string(arg); });

	// Reteriving the pid of the last running sensor
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
			//write_to(current_dir+"/.cmd","stop",false);
			remove_sensor_pid(sensor_name, run_path);
			kill_sensor(pid);
			return 0;
		} else {
			std::cout << "The sensor is NOT running.\n";
			return 0;
		}
	}
	else if ((iter = std::find(args.begin(), args.end(), "status")) != args.end()) {
		if (pid != -1) {
			// pass the command stop
			std::cout << "Gathering info about the sensor pid= " << pid << "\n";
				iter = std::find(args.begin(), args.end(), "-h");
				if (iter == args.end()) {
					// debug mode
					// measure how many events the Linux generates
					std::cout << "You must specify the address and port to connect to the sensor\n";
					return 0;
				}
				else {
					std::string value = *(iter + 1);
					auto split = value.find(':');

					std::string hostname = value.substr(0, split);
					std::string port = value.substr(split + 1);

					linux_socket socket(hostname.c_str(), std::atoi(port.c_str()));
					if (!socket.connect()) {
						std::cout << "The sensor is not running in logging mode.\n";
						return 0;
					}
					std::string reciv;
					do {
						reciv = socket.receive();
						if (reciv.empty())
							break;
						std::cout << reciv << "\n";
					} while (true);

				}

		} else {
			std::cout << "The sensor is NOT running.\n";
			return 0;
		}
	}
	else if ((iter = std::find(args.begin(), args.end(), "start")) != args.end()) {
		sensor_stream<sys_record> *stream = setup_stream<sys_record>(args);
		if (pid ==-1) {
			setup_daemon(sensor_name, run_path);
			iter = std::find(args.begin(), args.end(), "-r");
			if (iter != args.end()) {
				// debug mode
				// measure how many events the Linux generates
			}
			else {
				multithreaded_sensor<sys_record> linux_sensor;
				linux_sensor.config_events(true, false);
				iter = std::find(args.begin(), args.end(), "-l");
				if (iter != args.end()) {
					linux_sensor.run(true);
				}else{
					linux_sensor.run(false);
				}
			}
		}
		else{
			std::cout << "The sensor is running.\n";
			return 0;
		}
	}
}
