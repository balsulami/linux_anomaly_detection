
#pragma once

#include"format.h"
#include "string_ref.h"
#include<string>


class SysRecord {
public:
	SysRecord(pid_t pid = 0,syscall_t syscall = 0) :_pid(pid), _syscall(syscall){}

	SysRecord(StringRef & pid, StringRef & syscall):SysRecord()
	{
		set_pid(pid);
		set_syscall(syscall);
	}

	std::string to_string(){
		std::string json = fmt::format("'{0}','{1}'", _pid,_syscall);
		return json;
	}

	void set_pid(StringRef & pid){
		auto begin = pid.begin();
		while (begin != pid.end()) {
			if (*begin == ' ')
				break;
			_pid = _pid * 10 + (*begin - '0');
			++begin;
		}
	}

	void set_syscall(StringRef & syscall_id){
		auto begin = syscall_id.begin();
		while (begin != syscall_id.end()) {
			if (*begin == ' ')
				break;
			_syscall = _syscall * 10 + (*begin - '0');
			++begin;
		}
	}

	pid_t _pid;
	syscall_t _syscall;
};