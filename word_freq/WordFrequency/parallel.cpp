#include "parallel.h"

#include <map>
#include <vector>
#include <string>
#include <functional>

using namespace std;

namespace parallel
{
	// keep these details private
	struct Worker::Data
	{
		vector<string> const& mData;

		const unsigned int mBatchBeginOffset;
		const unsigned int mBatchEndOffset;

		Worker::WorkerResult_t mDict;

		volatile bool mFinished;

		Data(vector<string> const& data, unsigned int beginOffset, unsigned int endOffset);
	};

	Worker::Data::Data(vector<string> const& data, unsigned int beginOffset, unsigned int endOffset)
		: mData(data)
		, mBatchBeginOffset(beginOffset)
		, mBatchEndOffset(endOffset)
		, mFinished(false)
	{}

	Worker::Worker(thread* thr, Data* data)
		: mThread(thr)
		, mThreadData(data)
	{}

	Worker::~Worker()
	{
		mThread->join();

		mThreadData->mDict.clear();
		delete mThreadData;
		delete mThread;
	}

	Worker* Worker::createWorker(vector<string> const& rawData, unsigned int offset, unsigned int count)
	{
		Data* workerData = new Data(rawData, offset, offset + count);
		thread* workerThread = new thread(threadFn, workerData);
		Worker* worker = new Worker(workerThread, workerData);
		return worker;
	}

	bool Worker::isProcessing() const
	{
		return !mThreadData->mFinished;
	}

	void Worker::collateResults(WorkerResult_t& results) const
	{
		for (auto it = mThreadData->mDict.cbegin(); it != mThreadData->mDict.cend(); ++it)
		{
			auto findItr = results.find(it->first);
			if (findItr != results.cend())
			{
				findItr->second.second += it->second.second;
			}
			else
			{
				typedef pair<WorkerResult_t::key_type, WorkerResult_t::mapped_type> _insert_t;
				results.insert(_insert_t(it->first, WorkerResult_t::mapped_type(it->second.first, it->second.second)));
			}
		}
	}

	void Worker::threadFn(Data* threadData)
	{
		hash<string> hasher;

		for (unsigned int i = threadData->mBatchBeginOffset; i < threadData->mBatchEndOffset; ++i)
		{
			string const& str = threadData->mData[i];
			size_t strHash = hasher(str);

			auto it = threadData->mDict.find(strHash); // <-- usually not a big fan of auto, but good for verbose iterators
			if (it != threadData->mDict.cend())
			{
				// should either deal with hash collision, or assert that string matches
				it->second.second++;
			}
			else
			{
				typedef pair<WorkerResult_t::key_type, WorkerResult_t::mapped_type> _insert_t;
				threadData->mDict.insert(_insert_t(strHash, WorkerResult_t::mapped_type(str, 1)));
			}
		}

		threadData->mFinished = true;
	}
}
