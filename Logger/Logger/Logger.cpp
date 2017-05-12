// Logger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Logger.h"

void write(int id)
{
	int i = 0;
	while (i != 100){
		auto message = "thread ¹" + std::to_string(id) + "\tcounter: " + std::to_string(i);
		if (i % 5 == 0){
			Logger::GetInstance().Info("Sss", message, "first", "second");
		}
			i++;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	Logger::GetInstance().SetLogFile("testlog.log");
	Logger::GetInstance().SetLogLevel(LOG_INFO);
	std::thread thread1(std::bind(&write, 1));
	/*std::thread thread2(std::bind(&write, 2));
	std::thread thread3(std::bind(&write, 3));
	std::thread thread4(std::bind(&write, 4));
	std::thread thread5(std::bind(&write, 5));*/

	thread1.join();
	/*thread2.join();
	thread3.join();
	thread4.join();
	thread5.join();*/
	system("pause");
	return 0;
}

