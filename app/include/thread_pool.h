#pragma once

#include "thread_safe_queue.h"

namespace Test
{

class ThreadPool
{
private:
	class Joiner
	{
	public:
		Joiner(std::vector<std::thread>& threads)
			: m_threadsRef(threads)
		{
		}

		~Joiner()
		{
			for (auto& thread : m_threadsRef)
			{
				if (!thread.joinable())
				{
					continue;
				}

				thread.join();
			}
		}

	private:
		std::vector<std::thread>& m_threadsRef;
	};

	class TaskWrapper
	{
	private:
		struct ImplBase
		{
			virtual void invoke() = 0;
			virtual ~ImplBase() = default;
		};

		template <typename F>
		struct Impl : ImplBase
		{
			Impl(F&& f)
				: f(std::move(f))
			{
			}

			virtual void invoke() override
			{
				f();
			}

			F f;
		};

	public:
		TaskWrapper() = default;

		template <typename F>
		TaskWrapper(F&& f)
			: m_impl(std::make_unique<Impl<F>>(std::forward<F>(f)))
		{
		}

		template <typename F>
		TaskWrapper& operator=(F&& f)
		{
			m_impl = std::make_unique<Impl<F>>(std::forward<F>(f));
		}

		void operator()() const
		{
			m_impl->invoke();
		}

	private:
		std::unique_ptr<ImplBase> m_impl;
	};

public:
	ThreadPool()
		: m_done(false)
		, m_joiner(m_threads)
	{
		try
		{
			for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i)
			{
				m_threads.emplace_back(&ThreadPool::workerThreadEntryPoint, this);
			}
		}
		catch (...)
		{
			m_done = true;
			throw;
		}
	}

	~ThreadPool()
	{
		m_done = true;
	}

	template <typename F>
	std::future<typename std::result_of<F()>::type> pushTask(F&& f)
	{
		using ReturnType = typename std::result_of<F()>::type;

		std::packaged_task<ReturnType()> packagedTask(f);
		std::future<ReturnType> future(packagedTask.get_future());

		m_queue.push(TaskWrapper(std::move(packagedTask)));

		return future;
	}

private:
	void workerThreadEntryPoint()
	{
		while (!m_done)
		{
			TaskWrapper task;

			if (!m_queue.pop(task))
			{
				std::this_thread::yield();

				continue;
			}

			task();
		}
	}

private:
	std::atomic_bool m_done;
	ThreadSafeQueue<TaskWrapper> m_queue;
	std::vector<std::thread> m_threads;
	Joiner m_joiner;
};

}