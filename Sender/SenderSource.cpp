#include <fstream>
#include <iostream>
#include <string>
#include <Windows.h>

#define MAX_SIZE 20

int main(int argc, char** argv) 
{
	std::cout << "Hi, I am sender " << std::stoi(argv[1]) << '\n';


	HANDLE hIAmReady = (HANDLE)std::stoi(argv[3]);
	HANDLE hMutexFile = (HANDLE)std::stoi(argv[4]);

	std::ofstream fout;
	std::cout << "Trying to open file: " << argv[2] << '\n';

	fout.open(argv[2], std::ios_base::trunc);

	if (!fout.is_open()) 
	{
		std::cerr << "Failed to open file: " << argv[2];
		return 1;
	}

	if (!SetEvent(hIAmReady))
	{
		std::cout << "I can't say that I am ready! Heelp! Error: " << GetLastError();
	}

	fout.close();
	while (true) 
	{
		int option = 0;
		std::cout << "What should I do?\n1. Send message\n2. End work\n";
		std::cin >> option;
		switch (option)
		{
		case 1:
		{
			std::string message;
			std::cout << "Please, enter message:\n";

			std::cin.ignore();
			std::getline(std::cin, message);

			fout.open(argv[2], std::ios_base::binary | std::ios_base::app);

			char messageFormatted[MAX_SIZE];
			memset(messageFormatted, 0, sizeof(messageFormatted));

			int length = 0;
			if (message.size() < MAX_SIZE)
			{
				length = message.size();
			}
			else 
			{
				length = MAX_SIZE;
			}

			for (size_t i = 0; i < length; i++)
			{
				messageFormatted[i] = message[i];
			}

			WaitForSingleObject(hMutexFile, INFINITE);

			fout.write(messageFormatted, sizeof(messageFormatted));

			fout.flush();

			ReleaseMutex(hMutexFile);

			fout.close();

			break;
		}
		case 2:
		{
			return 0;
			break;
		}
		default:
			std::cout << "Sorry I cannot understand your command :( Bye-Bye!\n";
			system("pause");
			return 1;
		}
	}

	system("pause");
	return 0;
}