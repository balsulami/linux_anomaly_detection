//
// Created by root on 4/17/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_UTILS_H
#define HIGHPERFORMANCELINUXSENSORS_UTILS_H
#include "config.h"
#include<fstream>
#include<string>
#include<thread>
#include<chrono>
#include <unistd.h>
#include <signal.h>

using std::ifstream;

void write_to(const std::string & filename, const std::string & content, bool append = true){
    std::ios_base::openmode mode = std::ios::binary;
    if(append)
        mode |= std::ios::app;
    std::ofstream outfile(filename.data(), mode);
    outfile << content << "\n";
    outfile.close();
}

static inline std::vector<StringRef> split_string(std::string& str,
	const std::string& delimiter, std::string& last_line)
{
	std::vector<StringRef> strings;
	strings.reserve(200);

	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while ((pos = str.find(delimiter, prev)) != std::string::npos)
	{
		strings.emplace_back(str.begin() + prev, pos - prev);
		prev = pos + 1;
	}


	if (prev < str.size()-1){
		//std::string prevs(str.begin() + prev,str.end());
		last_line.append(str.begin() + prev, str.end());
		str.resize(prev);
	}
	return strings;
}

short str_to_short(std::string::const_iterator begin, std::string::const_iterator end){
    short _pid = 0;
    while (begin != end) {
        _pid = _pid * 10 + (*begin - '0');
        ++begin;
    }
    return _pid;
}

pid_t get_pid(){
	return getpid();
}

std::vector<pid_t> get_pids(pid_t ppid){
    std::vector<pid_t> pids;
    FILE* pipe = popen((std::string("ls -l /proc/")+ std::to_string(ppid) + "/task").c_str(), "r");
    if (!pipe) return pids;
    char buffer[128];

    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL) {
            std::string result = buffer;
            if(result.size() > 10){
                pids.push_back(str_to_short(result.begin()+result.find_last_of(" ")+1,result.end()-1));
            }
        }

    }
    pclose(pipe);
    return pids;
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
    auto curr_pid = get_pid();
    auto pids = get_pids(pid);
    for (auto itr = pids.begin();itr < pids.end(); itr++){
        if(curr_pid != *itr)
            kill(*itr, SIGKILL);
    }
    kill(pid, SIGKILL);
}

#endif //HIGHPERFORMANCELINUXSENSORS_UTILS_H
