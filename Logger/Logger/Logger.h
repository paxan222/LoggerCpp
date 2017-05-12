#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include "stdarg.h"

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

enum LogLevel
{
	LOG_QUIET = -1,
	LOG_INFO = 0,
	LOG_ERROR,
	LOG_DEBUG,
	LOG_TRACE
};

class Logger
{
	LogLevel default_level = LOG_ERROR;

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

	void WriterThread()
	{
		writer_thread_ = std::thread([this]{
			while (!quit_){
				if (file == nullptr)
					return;
				std::string message = message_queue_.pop();
				fputs(message.c_str(), file);
				fflush(file);
			}
		});
		writer_thread_.joinable();
	}

	Logger()
	{

	}

	~Logger()
	{
		quit_ = true;
		writer_thread_.join();
		if (file != nullptr){
			fclose(file);
		}
	}

	Logger(Logger const&) = delete;

	Logger& operator = (Logger const&) = delete;
public:

	static Logger& GetInstance()
	{
		static Logger logger;
		return logger;
	}

	void SetLogFile(std::string filename)
	{
		file = fopen(filename.c_str(), "a+");
		if (file != nullptr)
			WriterThread();
	}

	void SetLogLevel(LogLevel level)
	{
		default_level = level;
	}

	void PushToQueue(LogLevel log_level, va_list list, const char* format)
	{
		std::string output_message = CurrentDateTime();
		switch (log_level){
			case LOG_QUIET:
				return;
			case LOG_INFO:
				output_message += "|INFO|";
				break;
			case LOG_ERROR:
				output_message += "|ERROR|";
				break;
			case LOG_DEBUG:
				output_message += "|DEBUG|";
				break;
			case LOG_TRACE:
				output_message += "|TRACE|";
				break;
			default:
				break;
		}
		while (*format != '\0'){
			switch (*format){
				case 'i':
					output_message += std::to_string(va_arg(list, int)) + "|";
					format++;
					break;
				case 'd':
					output_message += std::to_string(va_arg(list, double)) + "|";
					format++;
					break;
				case 'f':
					output_message += std::to_string(va_arg(list, float)) + "|";
					format++;
					break;
				case 'c':
					output_message += va_arg(list, char) + "|";
					format++;
					break;
				case 's':
					output_message += static_cast<std::string>(va_arg(list, char *)) + "|";
					format++;
					break;
				case 'S':
					output_message += va_arg(list, std::string) + "|";
					format++;
					break;
				default:
					format++;
					break;
			}
		}
		va_end(list);
		output_message += "\n";
		message_queue_.push(output_message);
	}

	void Info(const char* format, std::string ...)
	{
		if (default_level != LOG_INFO)
			return;
		va_list list;
		va_start(list, format);
		PushToQueue(LOG_INFO, list, format);
		/*std::string output_message = CurrentDateTime() + "|INFO|";
		va_start(list, format);
		while (*format != '\0'){
		switch (*format){
		case 'i':
		output_message += std::to_string(va_arg(list, int)) + "|";
		format++;
		break;
		case 'd':
		output_message += std::to_string(va_arg(list, double)) + "|";
		format++;
		break;
		case 'f':
		output_message += std::to_string(va_arg(list, float)) + "|";
		format++;
		break;
		case 'c':
		output_message += va_arg(list, char) + "|";
		format++;
		break;
		case 's':
		output_message += static_cast<std::string>(va_arg(list, char *)) + "|";
		format++;
		break;
		case 'S':
		output_message += va_arg(list, std::string) + "|";
		format++;
		break;
		default:
		format++;
		break;
		}
		}
		va_end(list);
		output_message += "\n";
		message_queue_.push(output_message);*/

	}

	void Error(const char* format, std::string ...)
	{
		if (default_level != LOG_ERROR)
			return;

		std::string output_message = CurrentDateTime() + "|ERROR|";
		va_list list;
		int i;
		va_start(list, format);
		while (*format != '\0'){
			switch (*format){
				case 'i':
					output_message += std::to_string(va_arg(list, int)) + "|";
					format++;
					break;
				case 'd':
					output_message += std::to_string(va_arg(list, double)) + "|";
					format++;
					break;
				case 'f':
					output_message += std::to_string(va_arg(list, float)) + "|";
					format++;
					break;
				case 'c':
					output_message += va_arg(list, char) + "|";
					format++;
					break;
				case 's':
					output_message += static_cast<std::string>(va_arg(list, char *)) + "|";
					format++;
					break;
				case 'S':
					output_message += va_arg(list, std::string) + "|";
					format++;
					break;
				default:
					format++;
					break;
			}
		}
		va_end(list);
		output_message += "\n";

		message_queue_.push(output_message);

	}

	void Debug(const char* format, std::string ...)
	{
		if (default_level != LOG_DEBUG)
			return;

		std::string output_message = CurrentDateTime() + "|DEBUG|";
		va_list list;
		int i;
		va_start(list, format);
		while (*format != '\0'){
			switch (*format){
				case 'i':
					output_message += std::to_string(va_arg(list, int)) + "|";
					format++;
					break;
				case 'd':
					output_message += std::to_string(va_arg(list, double)) + "|";
					format++;
					break;
				case 'f':
					output_message += std::to_string(va_arg(list, float)) + "|";
					format++;
					break;
				case 'c':
					output_message += va_arg(list, char) + "|";
					format++;
					break;
				case 's':
					output_message += static_cast<std::string>(va_arg(list, char *)) + "|";
					format++;
					break;
				case 'S':
					output_message += va_arg(list, std::string) + "|";
					format++;
					break;
				default:
					format++;
					break;
			}
		}
		va_end(list);
		output_message += "\n";

		message_queue_.push(output_message);

	}

	void Trace(const char* format, std::string ...)
	{
		if (default_level != LOG_DEBUG)
			return;

		std::string output_message = CurrentDateTime() + "|TRACE|";
		va_list list;
		int i;
		va_start(list, format);
		while (*format != '\0'){
			switch (*format){
				case 'i':
					output_message += std::to_string(va_arg(list, int)) + "|";
					format++;
					break;
				case 'd':
					output_message += std::to_string(va_arg(list, double)) + "|";
					format++;
					break;
				case 'f':
					output_message += std::to_string(va_arg(list, float)) + "|";
					format++;
					break;
				case 'c':
					output_message += va_arg(list, char) + "|";
					format++;
					break;
				case 's':
					output_message += static_cast<std::string>(va_arg(list, char *)) + "|";
					format++;
					break;
				case 'S':
					output_message += va_arg(list, std::string) + "|";
					format++;
					break;
				default:
					format++;
					break;
			}
		}
		va_end(list);
		output_message += "\n";

		message_queue_.push(output_message);

	}

	void WriteLine(std::string message)
	{
		std::string output_message = "";
		output_message = CurrentDateTime() + "\t" + message + "\n";
		message_queue_.push(output_message);
	}
};