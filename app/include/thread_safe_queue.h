#pragma once

namespace Test
{

template <typename T>
class ThreadSafeQueue
{
public:
	ThreadSafeQueue() = default;

	void push(const T& object)
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		m_queue.push(object);
	}

	void push(T&& object)
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		m_queue.push(std::move(object));
	}

	bool pop(T& targetObject)
	{
		std::lock_guard<std::mutex> locker(m_mutex);

		if (m_queue.empty())
		{
			return false;
		}

		targetObject = std::move(m_queue.front());

		m_queue.pop();

		return true;
	}

private:
	mutable std::mutex m_mutex;
	std::queue<T> m_queue;
};

}