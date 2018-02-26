#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include "textreader.h"
#include "wordfreq.h"

using namespace std;

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		cerr << "usage: WordFrequency.exe <filename>" << endl;
		exit(-1);
	}

	ifstream infile(argv[1]);

	if (!infile.good())
	{
		cerr << "error: could not find file \"" << argv[1] << "\"" << endl;
		exit(-1);
	}

	// NOTE: general assumptions and reasoning after looking at sample data
	// lines contain 1 or more complete words
	//   no lines contain partial words
	//   lines are delimited by "\r\n"
	// effective "egalitarian" parallel processing of the data is not going to be easy in the alloted time
	// (2 hrs), because lines are a mix of 1, several or many words
	// if time allows, my approach in this will be to read the file in reasonably even chunks, 
	// identify word start boundaries to determine job limits, then task the threads with 
	// grinding through those instead

	// begin processing
	vector<string>* vec = new vector<string>();
	// premature optimization to speed up file read into vector
	vec->reserve((2 << 20) / 8); // initial storage estimate: 1MiB worth of 8-letter words...

	// read in text file as vector of strings
	util::parseFile(infile, *vec);

	if (vec->size() == 0)
	{
		// no strings to process
		exit(0);
	}

	vector<wordfreq::WordFrequency_t>* results = new vector<wordfreq::WordFrequency_t>();
	results->reserve(vec->size());

	// process vector of strings
	wordfreq::determineWordFrequency(*vec, *results);

	//   report results
	for (auto itr = results->cbegin(); itr != results->cend(); ++itr)
	{
		cout << setw(20) << left << itr->first << setw(10) << left << "count: " << itr->second << endl;
	}

	// cleanup
	delete vec;
	delete results;

	// future?
	//   investigate trie as option (map is faster option for homework)

	return 0;
}

