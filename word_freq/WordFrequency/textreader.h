#include <fstream>
#include <vector>
#include <string>

namespace util
{
	// there were going to be other "helpful" utils here, but for this simple implementation
	// it proved unnecessary
	void parseFile(std::ifstream& file, std::vector<std::string>& words);
}
