#pragma once
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <deque>
#include <vector>
#include <thread>

class Job;
class JobWorker;

enum class JobType
{
	GENERIC,      // Default / unspecified job
	PHYSICS,      // Jobs related to physics updates (e.g., ground checks, collisions)
	AI,           // AI-related tasks (e.g., path finding, decision-making)
	RENDERING,    // Rendering-related jobs (e.g., mesh processing, LOD updates)
	AUDIO,        // Audio processing jobs
	NETWORKING,   // Network-related tasks (e.g., sending/receiving data)
	IO,           // File reading/writing jobs
};

enum class JobStatus
{
	NEW, // Constructed by the main thread
	QUEUE, // Owned by the Job System, waiting to be claimed by a worker
	EXECUTING, // Claimed & owned by worker who is executing it
	COMPLETED, // Completed & placed in list for main thread to retrieve
	RETRIEVED // Only the main thread controls everything. The main thread retrieves the jobs retired from the Job System
};

struct JobSystemConfig
{
	int m_numWorkers = -1; // If negative, automatically creates 1 worker 
};

class JobSystem
{
public:
	JobSystem(JobSystemConfig config);
	~JobSystem();

	void Startup();
	void ShutDown();
	void BeginFrame();
	void EndFrame();

	void QueueJob(Job* jobToQueue);
	void WaitForAllJobsToFinish();
	Job* ClaimJob();
	Job* ClaimJobForWorker(JobWorker& worker);
	void RetrieveCompletedJob(Job* job);
	Job* RetrieveCompletedJob();
	void RetrieveCompletedJobs(std::vector<Job*>& outJobs, int maxJobs);
	void SubmitCompleteJob(Job* job);

	void CreateWorkers(int numWorkers);
	void DeleteWorkers(); // Deconstructor calls this 

	int GetNumQueuedJobs() const;

public:
	std::atomic<bool> m_isShuttingDown = false;
	std::condition_variable m_condition;

	mutable std::mutex m_queueMutex;
	mutable std::mutex m_executingMutex;
	mutable std::mutex m_completedMutex;

	std::vector<Job*> m_allJobs;
	std::deque<Job*> m_queueJobs;
	std::deque<Job*> m_executingJobs;
	std::deque<Job*> m_completedJobs;

	std::vector<JobWorker*> m_workers;
	JobSystemConfig m_config;
};

class Job
{
public:
	Job(JobType type) : m_type(type) {}
	Job() = default;
	virtual ~Job() = default;
	
	virtual void Execute() = 0;

	JobType GetJobType() const { return m_type; }

	std::atomic<JobStatus> m_state = JobStatus::NEW;

private:
	JobType m_type;
};

class JobWorker
{
public:
	JobWorker(int id, JobSystem* jobSystem);
	void ThreadMain();

	friend class JobSystem;

private:
	int m_jobWorkerID;
	std::thread* m_thread = nullptr;
	JobSystem* m_jobSystem = nullptr;
};