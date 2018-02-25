#pragma once

#include <thread>
#include <map>
#include <vector>
#include <functional>
#include <string>

namespace parallel
{
	// specific use-case worker thread for word-frequency calculation
	class Worker
	{
		struct Data;

		std::thread* mThread;
		Data* mThreadData;

	public:
		typedef std::map<std::size_t, std::pair<std::string, unsigned int> > WorkerResult_t;

		~Worker();

		bool isProcessing() const;

		void collateResults(WorkerResult_t& results) const;

		static Worker* createWorker(std::vector<std::string> const& data, unsigned int offset, unsigned int count);

	private:
		Worker(std::thread* thr, Data* data);

		static void threadFn(Data* threadData);
	};
}
