
#pragma once

#include<vector>
#include"utils/string_ref.h"

class EventsBulk{
public:
	EventsBulk(std::string & buffer, std::vector<StringRef> & lines) :_buffer(std::move(buffer)),_lines(std::move(lines)){
	}

	std::vector<StringRef> & lines(){ return _lines; }
	std::string & buffer(){ return _buffer; }

	~EventsBulk(){
	}

private:
	std::string _buffer;
	std::vector<StringRef> _lines;
};