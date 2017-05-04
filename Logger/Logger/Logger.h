#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>

template<typename T> class ThreadSafeQueue
{
	std::condition_variable condition_variable_;
	std::mutex mutex_;
	std::queue<T> queue_;

public:

	bool empty()
	{
		return queue_.empty();
	}

	void push(T& value)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		queue_.push(value);
		lock.unlock();
		condition_variable_.notify_one();
	}

	T pop()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()){
			condition_variable_.wait(lock);
		}
		auto value = queue_.front();
		queue_.pop();
		lock.unlock();
		return value;
	}

	T front()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()){
			condition_variable_.wait(lock);
		}
		auto value = queue_.front();
		lock.unlock();
		return value;
	}

	T back()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (queue_.empty()){
			condition_variable_.wait(lock);
		}
		auto value = queue_.back();
		lock.unlock();
		return value;
	}

	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
};

class Logger
{
	FILE* file;
	ThreadSafeQueue<std::string> message_queue_;
	bool quit_{ false };
	std::thread writer_thread_;

	std::string CurrentDateTime() const
	{
		auto t = time(nullptr);
		auto now = localtime(&t);
		auto current_date_time =
			std::to_string(now->tm_year + 1900) + "-" +
			std::to_string(now->tm_mon + 1) + "-" +
			std::to_string(now->tm_mday) + " " +
			std::to_string(now->tm_hour) + "-" +
			std::to_string(now->tm_min) + "-" +
			std::to_string(now->tm_sec);
		return current_date_time;
	}

	void WriteToFile(FILE* file, ThreadSafeQueue<std::string>& queue)
	{
		std::string message = queue.pop();
		fputs(message.c_str(), file);
		fflush(file);
	}

	void CreateWriterThread()
	{
		writer_thread_ = std::thread([this]{
			while (!quit_){
				WriteToFile(file, std::ref(message_queue_));
			}
			while (!message_queue_.empty()){
				WriteToFile(file, std::ref(message_queue_));
			}
		});
		writer_thread_.joinable();
	}
public:
	Logger(std::string filename)
	{
		file = fopen(filename.c_str(), "w");
		if (file != nullptr)
			CreateWriterThread();
	}
	~Logger()
	{
		quit_ = true;
		writer_thread_.join();
		if (file != nullptr){
			fclose(file);
		}
	}

	void Write(std::string message)
	{
		auto output_message ="\n" + CurrentDateTime() + "\t" + message;
		message_queue_.push(output_message);
	}
};