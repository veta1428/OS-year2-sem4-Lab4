#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>
#include <vector>

#define MAX_LENGTH 20

int main() 
{
	std::cout << "Enter file name: ";
	std::string filename;
	std::cin >> filename;

	std::ifstream fin;
	fin.open(filename, std::ios_base::binary);

	if (!fin.is_open()) {
		std::cerr << "Cannot open file: " << filename << "\n";
		return 1;
	}

	std::cout << "Enter the number of senders: ";
	int senders = 0;
	std::cin >> senders;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    std::vector<HANDLE> sendersHandles;
    std::vector<HANDLE> sendersThreadHandles;
    std::vector<HANDLE> hReadies;

    const size_t cSize = strlen(filename.c_str()) + 1;
    wchar_t* wfilename = new wchar_t[cSize];
    mbstowcs(wfilename, filename.c_str(), cSize);

    SECURITY_ATTRIBUTES mutexSecurity;
    mutexSecurity.bInheritHandle = TRUE;
    mutexSecurity.lpSecurityDescriptor = NULL;
    mutexSecurity.nLength = sizeof(SECURITY_ATTRIBUTES);

    HANDLE hMutexFile = CreateMutex(&mutexSecurity, FALSE, NULL);

    for (int i = 0; i < senders; i++)
    {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        SECURITY_ATTRIBUTES securityReady;
        securityReady.bInheritHandle = TRUE;
        securityReady.lpSecurityDescriptor = NULL;
        securityReady.nLength = sizeof(SECURITY_ATTRIBUTES);

        HANDLE hReady = CreateEvent(
            &securityReady,           
            FALSE,  
            FALSE, 
            NULL);

        wchar_t buffer[1000];

        wsprintfW(buffer, L"Sender.exe %d %s %d %d", i, wfilename, hReady, hMutexFile);

        if (!CreateProcess(NULL,   // No module name (use command line)
            buffer,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            TRUE,          // Set handle inheritance to FALSE
            CREATE_NEW_CONSOLE,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            std::cerr << "Creating sender #" << i << " failed. Error: " << GetLastError();
            return 1;
        }

        sendersHandles.push_back(pi.hProcess);
        sendersThreadHandles.push_back(pi.hThread);
        hReadies.push_back(hReady);
    }

    //waiting when senders are ready
    int result = 0;
    WaitForMultipleObjects(senders, &hReadies[0], TRUE, INFINITE);
    fin.close();
    int counter = 0;
    while (true)
    {
        int option = 0;
        std::cout << "What should I do?\n1. Read message\n2. End work\n";
        std::cin >> option;
        switch (option)
        {
        case 1:
        {
            char messageFormatted[MAX_LENGTH];

            WaitForSingleObject(hMutexFile, INFINITE);

            fin.open(filename, std::ios_base::binary);
            fin.seekg(counter * sizeof(messageFormatted), std::ios_base::beg);
            fin.read(messageFormatted, sizeof(messageFormatted));

            fin.close();

            ReleaseMutex(hMutexFile);

            if (fin.eof()) {
                std::cout << "Sorry, no unread messages!\n";
            }
            else {
                std::cout << "Read message: " << messageFormatted << "\n";
                counter++;
            }

            break;
        }
        case 2:
        {
            fin.close();

            for (size_t i = 0; i < senders; i++)
            {
                CloseHandle(sendersHandles[i]);
                CloseHandle(sendersThreadHandles[i]);
                CloseHandle(hReadies[i]);
            }

            CloseHandle(hMutexFile);
            delete[] wfilename;

            return 0;
            break;
        }
        default:
            std::cout << "Sorry I cannot understand your command :(\n";
            break;
        }
    }

    std::cout << "\n well, all senders are ready";

	return 0;
}