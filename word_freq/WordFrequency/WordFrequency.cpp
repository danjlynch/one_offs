#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

#include "textreader.h"
#include "parallel.h"

using namespace std;

int main(int argc, const char** argv)
{
    if (argc != 2)
    {
        exit(-1);
    }

	ifstream infile(argv[1]);

	if (!infile.good())
	{
		exit(-1);
	}

	// NOTE: general assumptions and reasoning after looking at sample data
	// lines contain 1 or more complete words
	//   no lines contain partial words
	//   lines are delimited by "\r\n"
	// "egalitarian" parallel processing of the data is not going to be easy in the alloted time
	// (2 hrs), because lines are a mix of 1, several or many words
	// if time allows, my approach in this will be to read the file in reasonaly even chunks, 
	// identify word start boundaries to determine job limits, then task the threads with 
	// grinding through those instead

    // prepare systems
    //   prepare work queue

	parallel::Worker::WorkerResult_t* result = new parallel::Worker::WorkerResult_t();

    // begin processing
	vector<string>* vec = new vector<string>();
	// premature optimization to speed up file read into vector
	vec->resize((2<<20) / 8); // 1MiB worth of 8-letter words...

	util::parseFile(infile, *vec);

	if (vec->size() == 0)
	{
		// no strings to process
		exit(0);
	}
    
    // MT processing
	//   prepare worker threads
	const unsigned int kNumWorkers = 8; // hardcode a worker pool of 4 threads for now
	const unsigned int kBatchSize = (vec->size() / 8);
	const unsigned int kBatchSizeOverrun = (vec->size() % 8);
	 
	// parallelised by operating on batches of vector entries, using interlocked exchange of long 
	// to determine next working offset into vector
	parallel::Worker** workers = new parallel::Worker*[kNumWorkers]
	{
		parallel::Worker::createWorker(*vec, 0, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize * 2, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize * 3, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize * 4, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize * 5, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize * 6, kBatchSize),
		parallel::Worker::createWorker(*vec, kBatchSize * 7, kBatchSize + kBatchSizeOverrun)
	};

	// wait for workers
	unsigned int nProcessing;
	do
	{
		nProcessing = 0;
		for (unsigned int i = 0; i < kNumWorkers; ++i)
		{
			nProcessing += (workers[i]->isProcessing())
				? 1
				: 0;
		}
	}
	while (nProcessing > 0);

    // result
	//   collate results from workers
	for (unsigned int i = 0; i < kNumWorkers; ++i)
	{
		workers[i]->collateResults(*result);
	}

	//   sort results
    //   report results
	for (auto itr = result->cbegin(); itr != result->cend(); ++itr)
	{
		cout << "word: " << itr->second.first << "\t\tcount: " << itr->second.second << endl;
	}

	for (unsigned int i = 0; i < kNumWorkers; ++i)
	{
		delete workers[i];
	}

	delete workers;
	delete vec;
	delete result;

	// future?
	//   investigate trie as storage option (map is faster option for homework)

    return 0;
}

