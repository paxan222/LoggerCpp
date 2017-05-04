// Logger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Logger.h"

void write(Logger* logger, int id)
{
	int i = 0;
	while (i != 100){
		auto message = "thread ¹" + std::to_string(id) + "\tcounter: " + std::to_string(i);
		logger->Write(message);
		i++;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	Logger* logger = new Logger("testlog.txt");
	std::thread thread1(std::bind(&write, std::ref(logger), 1));
	std::thread thread2(std::bind(&write, std::ref(logger), 2));
	std::thread thread3(std::bind(&write, std::ref(logger), 3));
	std::thread thread4(std::bind(&write, std::ref(logger), 4));
	std::thread thread5(std::bind(&write, std::ref(logger), 5));

	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	thread5.join();
	system("pause");
	return 0;
}

