


#pragma  once
#include <string>
#include <vector>
class StringRef
{
private:
	std::string::const_iterator     begin_;
	size_t             size_;
public:
	size_t size() const { return size_; }
	std::string::const_iterator begin() const { return begin_; }
	std::string::const_iterator end() const { return begin_ + size_; }
//	std::string buff_;

	StringRef(const std::string::const_iterator & begin, const size_t size)
		: begin_(begin)
		, size_(size)
//		, buff_(begin, begin + size)
	{}

};


std::ostream & operator<<(std::ostream & out, const StringRef & ref){
	std::cout << std::string(ref.begin(), ref.end());
	return out;
}