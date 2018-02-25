#include "wordfreq.h"
#include "parallel.h"

#include <map>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std;

namespace
{
	// sort predicates for WordFrequency (ascending/descending, then alphabetically)
	struct {
		bool operator()(wordfreq::WordFrequency_t& a, wordfreq::WordFrequency_t& b) const
		{
			return a.second != b.second
				? a.second < b.second
				: a.first.compare(b.first) < 0;
		}
	} lowerFrequency;

	struct {
		bool operator()(wordfreq::WordFrequency_t& a, wordfreq::WordFrequency_t& b) const
		{
			return b.second != a.second
				? a.second > b.second
				: a.first.compare(b.first) < 0;
		}
	} higherFrequency;
}

namespace wordfreq
{
	void determineWordFrequency(std::vector<std::string> const& words, std::vector<WordFrequency_t>& frequencies)
	{
		// MT processing
		//   prepare worker threads
		const unsigned int kNumWorkers = 8; // hardcode a worker pool for now
		const unsigned int kBatchSize = (words.size() / 8);
		const unsigned int kBatchSizeOverrun = (words.size() % 8);

		// parallelised by operating on non-overlapping batches of vector entries
		parallel::Worker** workers = new parallel::Worker*[kNumWorkers]
		{
			parallel::Worker::createWorker(words, 0, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize * 2, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize * 3, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize * 4, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize * 5, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize * 6, kBatchSize),
				parallel::Worker::createWorker(words, kBatchSize * 7, kBatchSize + kBatchSizeOverrun)
		};

		// wait for workers
		unsigned int nProcessing;
		do
		{
			this_thread::sleep_for(chrono::milliseconds(10)); // I do *not* get this syntax... modern STL is weird...

			nProcessing = 0;
			for (unsigned int i = 0; i < kNumWorkers; ++i)
			{
				nProcessing += (workers[i]->isProcessing())
					? 1
					: 0;
			}
		} while (nProcessing > 0);

		// result
		//   collate results from workers
		parallel::Worker::WorkerResult_t* result = new parallel::Worker::WorkerResult_t();
		for (unsigned int i = 0; i < kNumWorkers; ++i)
		{
			workers[i]->collateResults(*result);
		}

		// copy all entries over to the results vector
		for (auto it = result->cbegin(); it != result->cend(); ++it)
		{
			frequencies.push_back(wordfreq::WordFrequency_t(it->second.first, it->second.second));
		}

		//   sort results before we return (most frequently occuring first)
		sort(frequencies.begin(), frequencies.end(), higherFrequency);

		// clean-up
		for (unsigned int i = 0; i < kNumWorkers; ++i)
		{
			delete workers[i];
		}
		delete workers;
		delete result;
	}
}
