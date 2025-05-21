#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

JobSystem* g_theJobSystem = nullptr;

JobSystem::JobSystem(JobSystemConfig config)
	: m_config(config)
{
}

JobSystem::~JobSystem()
{
	DeleteWorkers(); // May move to Shutdown function
}

void JobSystem::Startup()
{
	int numWorkers = m_config.m_numWorkers;
	if (numWorkers == -1)
	{
		numWorkers = std::thread::hardware_concurrency() - 1;
	}
	CreateWorkers(numWorkers);
}

void JobSystem::ShutDown()
{

}

void JobSystem::BeginFrame()
{
}

void JobSystem::EndFrame()
{
}

void JobSystem::QueueJob(Job* jobToQueue)
{
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		jobToQueue->m_state.store(JobStatus::QUEUE);
		m_queueJobs.emplace_back(jobToQueue);
		m_allJobs.emplace_back(jobToQueue);
	}
	m_condition.notify_one(); // Notify only one waiting thread
}

void JobSystem::WaitForAllJobsToFinish()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_condition.wait(lock, [this] { std::lock_guard<std::mutex> execLock(m_executingMutex); return m_queueJobs.empty() && m_executingJobs.empty(); });
}

Job* JobSystem::ClaimJob()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_condition.wait(lock, [this] {return !m_queueJobs.empty() || m_isShuttingDown; });

	if (m_isShuttingDown && m_queueJobs.empty())
	{
		return nullptr;
	}

	if (!m_queueJobs.empty())
	{
		Job* job = m_queueJobs.front();
		m_queueJobs.pop_front();
		job->m_state.store(JobStatus::EXECUTING);
		{
			std::lock_guard<std::mutex> executeLock(m_executingMutex);
			m_executingJobs.emplace_back(job);
		}
		return job;
	}
	return nullptr; // No Job found
}

Job* JobSystem::ClaimJobForWorker(JobWorker& worker)
{
	UNUSED(worker);
	std::lock_guard<std::mutex> lock(m_queueMutex);
	if (m_queueJobs.empty())
	{
		return nullptr;
	}

	Job* job = m_queueJobs.front();
	m_queueJobs.pop_front();
	job->m_state.store(JobStatus::EXECUTING);
	return job;
}

void JobSystem::RetrieveCompletedJob(Job* job)
{
	std::lock_guard<std::mutex> lock(m_completedMutex);
	auto it = std::find(m_completedJobs.begin(), m_completedJobs.end(), job);
	if (it != m_completedJobs.end())
	{
		job->m_state.store(JobStatus::RETRIEVED);
		m_completedJobs.erase(it);
	}
}

Job* JobSystem::RetrieveCompletedJob()
{
	std::lock_guard<std::mutex> lock(m_completedMutex);
	
	while (!m_completedJobs.empty())
	{
		Job* job = m_completedJobs.front();
		if (!job)
		{
			m_completedJobs.pop_front();
			continue;
		}

		JobStatus jobState = job->m_state.load(); // load reads value in a thread-safe way 

		if (job->m_state == JobStatus::NEW || static_cast<int>(jobState) < 0)
		{
			m_completedJobs.pop_front();
			continue;
		}

		m_completedJobs.pop_front();
		job->m_state.store(JobStatus::RETRIEVED);
		return job;
	}

	return nullptr;
}

void JobSystem::RetrieveCompletedJobs(std::vector<Job*>& outJobs, int maxJobs)
{
	std::lock_guard<std::mutex> lock(m_completedMutex);

	for (int i = 0; i < maxJobs && !m_completedJobs.empty(); i++)
	{
		outJobs.emplace_back(m_completedJobs.front());
		m_completedJobs.pop_front();
	}
}

void JobSystem::SubmitCompleteJob(Job* job)
{
	{
		std::lock_guard<std::mutex> executeLock(m_executingMutex);
		auto it = std::find(m_executingJobs.begin(), m_executingJobs.end(), job);
		if (it != m_executingJobs.end())
		{
			m_executingJobs.erase(it);
		}
	}
	{
		std::lock_guard<std::mutex> completedLock(m_completedMutex);
		job->m_state.store(JobStatus::COMPLETED);
		m_completedJobs.emplace_back(job);
	}
}

void JobSystem::CreateWorkers(int numWorkers)
{
	for (int i = 0; i < numWorkers; i++)
	{
		JobWorker* newWorker = new JobWorker(i, this);
		m_workers.emplace_back(newWorker);
	}
}

void JobSystem::DeleteWorkers()
{
	m_isShuttingDown = true;
	m_condition.notify_all(); // Notify all waiting threads

	for (JobWorker* worker : m_workers)
	{
		if (worker->m_thread->joinable())
		{
			worker->m_thread->join();
		}
		delete worker;
		worker = nullptr;
	}
	m_workers.clear();
}

int JobSystem::GetNumQueuedJobs() const
{
	std::lock_guard<std::mutex> lock(m_queueMutex);
	return static_cast<int>(m_queueJobs.size());
}

JobWorker::JobWorker(int id, JobSystem* jobSystem)
	: m_jobWorkerID(id), m_jobSystem(jobSystem)
{
	m_thread = new std::thread(&JobWorker::ThreadMain, this);
}

void JobWorker::ThreadMain()
{
	while (!m_jobSystem->m_isShuttingDown)
	{
		Job* jobToExecute = m_jobSystem->ClaimJob();
		if (jobToExecute)
		{
			jobToExecute->Execute();
			m_jobSystem->SubmitCompleteJob(jobToExecute);
		}
		else
		{
			std::this_thread::yield();
		}
	}
}