#pragma once

#include <string>
#include <vector>

namespace wordfreq
{
	typedef std::pair<std::string, unsigned int> WordFrequency_t;

	void determineWordFrequency(std::vector<std::string> const& words, std::vector<WordFrequency_t>& frequencies);
}
