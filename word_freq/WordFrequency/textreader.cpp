#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

namespace
{
	const char* kDelim = " ";

	void parseLine(string const& str, vector<string>& words)
	{
		// tokenise the line of text to break into individual words
		
		for (
			const char* token = strtok(const_cast<char*>(str.c_str()), kDelim);
			token != nullptr;
			token = strtok(nullptr, kDelim)
			)
		{
			if (strlen(token) > 0)
			{ 
				string str = string(token);
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				words.push_back(str);
			}
		}
	}
}

namespace util
{
	void parseFile(ifstream& infile, vector<string>& words)
	{
		words.clear();

		// read in the file, line by line, add lines to vector
		string line;
		while (getline(infile, line))
		{
			::parseLine(line, words);
		}

		infile.close();
	}
}
