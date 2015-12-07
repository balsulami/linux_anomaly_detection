//
// Created by root on 4/17/15.
//

#ifndef HIGHPERFORMANCELINUXSENSORS_UTILS_H
#define HIGHPERFORMANCELINUXSENSORS_UTILS_H

#include<fstream>
#include<string>
#include<thread>
#include<chrono>
#include <unistd.h>

using std::ifstream;

void write_to(const std::string & filename, const std::string & content, bool append = true){
    std::ios_base::openmode mode = std::ios::binary;
    if(append)
        mode |= std::ios::app;
    std::ofstream outfile(filename.data(), mode);
    outfile << content << "\n";
    outfile.close();
}

long find_last_linefeed(ifstream &infile) {

    infile.seekg(0,std::ios::end);
    long  filesize = infile.tellg();

    for(int n=1;n<filesize;n++) {
        infile.seekg(filesize-n-1,std::ios::beg);

        char c;
        infile.get(c);

        if(c == 0x0A) return infile.tellg();
    }
}

void sleep_sec(int sec){
	std::this_thread::sleep_for(std::chrono::seconds(sec));
}

inline std::vector<StringRef> split3(const std::string & str, char delimiter = ' ')
{
    std::vector<StringRef> result;
    result.reserve(20);

    enum State { inSpace, inToken };

    State state = inSpace;
    std::string::const_iterator     pTokenBegin = str.begin();    // Init to satisfy compiler.
    for (auto it = str.begin(); it != str.end(); ++it)
    {
        State newState = (*it == delimiter ? inSpace : inToken);
        if (newState != state)
        {
            switch (newState)
            {
                case inSpace:
                    result.emplace_back(pTokenBegin, std::abs(std::distance(it, pTokenBegin)));
                    break;
                case inToken:
                    pTokenBegin = it;
            }
        }
        state = newState;
    }
    if (state == inToken)
    {
        int ptr = std::abs(std::distance(pTokenBegin, str.end()));
        result.emplace_back(pTokenBegin, ptr);
    }
    return result;
}

int fast_atoi(std::string::const_iterator begin, std::string::const_iterator end)
{
    int val = 0;
    while (begin != end) {
        val = val * 10 + (*begin++ - '0');
    }
    return val;
}

static inline std::vector<StringRef> parse_trace(std::string& str,
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


	if (prev < str.size() - 1){
		std::string prevs(str.begin() + prev, str.end());
		last_line.append(str.begin() + prev, str.end());
		str.resize(prev);
	}
	return strings;
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

static inline std::vector<StringRef> split_fast(std::string& str,const std::string& delimiter)
{
	std::vector<StringRef> strings;
	strings.reserve(20);

	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while ((pos = str.find(delimiter, prev)) != std::string::npos)
	{
		strings.emplace_back(str.begin() + prev, pos - prev);
		prev = pos + 1;
	}

	if (prev < str.size())
		strings.emplace_back(str.begin() + prev, str.size());
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


#endif //HIGHPERFORMANCELINUXSENSORS_UTILS_H
