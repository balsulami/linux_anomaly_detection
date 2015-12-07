
#pragma once

#include"format.h"
#include "string_ref.h"
#include<string>
#include "sys_record.h"

class SysParamsRecord : public SysRecord {
public:
	SysParamsRecord() :SysRecord(){}

	SysParamsRecord(StringRef & name, StringRef & pid, StringRef & syscall, StringRef & arg) : SysRecord(pid,syscall),_name(name.begin(), name.end()), _arg(arg.begin(), arg.end())
	{

	}

	std::string to_string(){
		std::string json = fmt::format("'{0}','{1}','{2}','{3}'", _name,_pid,_syscall,_arg);
		return json;
	}


	std::string _name;
	std::string _arg;
};