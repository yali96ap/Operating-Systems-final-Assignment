#include "haifa.h"

char current_time[50];
int random;
int vesselsID[50];
HANDLE vessels[50];
HANDLE EilatReads, writeToEilat;      /* pipe for writing parent to child */
HANDLE HaifaReads, writeToHaifa;    /* pipe for writing child to parent */
DWORD read, written;
HANDLE vesselMutexes[50];
HANDLE printMutex;
void main(int argc, const char* argv[])
{
	srand(time(NULL));
	int converted;
	BOOL success, vesselsNumOk=0;
	
	TCHAR ProcessName[256];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	printMutex = CreateMutex(NULL, false, NULL);
	if (argc != 2)
	{
		fprintf(stderr, "You haven't entered a number of vessesls!\n");
		return;
	}
	int vesselNum = atoi(argv[1]);
	if (vesselNum < 2 || vesselNum > 50)
	{
		fprintf(stderr, "You have entered invalid number of vessels!\n");
		return;
	}
	for (int i = 0; i < vesselNum; i++)
	{
		vesselMutexes[i] = CreateMutex(NULL, false, NULL);
		WaitForSingleObject(vesselMutexes[i], INFINITE);
	}
	
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	/* create the pipe for writing from haifa to eilat */
	/* establish the START_INFO structure for the eilat process */
	GetStartupInfo(&si);

	/* redirect the standard input to the read end of the pipe */



	if (!CreatePipe(&EilatReads, &writeToEilat, &sa, 0)) {
		fprintf(stderr, " Haifa cant Create Pipe to eilat\n");
		return;
	}
	/* create the pipe for writing from eilat to haifa */
	if (!CreatePipe(&HaifaReads, &writeToHaifa, &sa, 0)) {
		fprintf(stderr, "Create Pipe from eilat to haifa failed\n");
		return;
	}
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si.hStdOutput = writeToHaifa;
	si.hStdInput = EilatReads;
	si.dwFlags = STARTF_USESTDHANDLES;


	wcscpy(ProcessName, L".\\eilatPort.exe");
	//mbstowcs_s(&converted, ProcessName, 256, ".\\eilatPort.exe", 256);
	if (!CreateProcessW(NULL, ProcessName, NULL, NULL, true, 0, NULL, NULL, &si, &pi))
	{
		fprintf(stderr, "Haifa: cant create Eilat process.\n");
		return;
	}


	// Read the number from the user
	int ves = atoi(argv[1]);

	/*haifa port now wants to write to the pipe to eilat  the number of vessels*/
	if (!WriteFile(writeToEilat, &ves, sizeof(int), &written, NULL))
	{
		fprintf(stderr, "Error writing to  Eilat pipe\n");
		return;
	}

	/* now read from the pipe */

	if (!ReadFile(HaifaReads, &vesselsNumOk, sizeof(bool), &read, NULL))
	{
		fprintf(stderr, "Haifa: Error reading from Eilat\n");
		return;

	}
	/* now check if its not prime */

	if (vesselsNumOk)
	{
		time_now(current_time);
		printf("%s Eilat accepts the number of vessels\n", current_time);

	}
	else
	{
		fprintf(stderr, "Eilat doesn't accepts the number of vessels\n");
		CloseHandle(HaifaReads);
		CloseHandle(writeToHaifa);
		return;
	}
	
	for (int i = 1; i <= vesselNum; i++)
	{
		
		vesselsID[i - 1] = i;
		vessels[i - 1] = CreateThread(NULL, 0, vesselRoute, &vesselsID[i - 1], 0, &converted);
		//WaitForSingleObject(vessels[i - 1], INFINITE);
	}


	for (int i = 1; i <= vesselNum; i++)
	{
		int vesselID;
		if (!ReadFile(HaifaReads, &vesselID, sizeof(int), &read, NULL))
		{
			fprintf(stderr, "Haifa: Error reading from Eilat\n");
			return;

		}
		ReleaseMutex(vesselMutexes[vesselID - 1]);

	}
	WaitForMultipleObjects(vesselNum, vessels, true, INFINITE);
	time_now(current_time);
	fprintf(stderr, "%s Haifa: All vessel threads are finished!\n", current_time);

	

	WaitForSingleObject(pi.hProcess, INFINITE);

	/* close the unused ends of the pipe */

	CloseHandle(writeToEilat);

	CloseHandle(EilatReads);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	for (int i = 0; i < vesselNum; i++)
	{
		CloseHandle(vesselMutexes[i]);
		CloseHandle(vessels[i]);
	}

	CloseHandle(HaifaReads);
	CloseHandle(writeToHaifa);
	time_now(current_time);
	CloseHandle(printMutex);
	printf("%s Haifa port: process finished\n", current_time);
}

	
void time_now(char* clock_time)
{
	time_t now = time(NULL);
	struct tm tnow;
	localtime_s(&tnow, &now);
	sprintf(clock_time, "[%02d:%02d:%02d]", tnow.tm_hour, tnow.tm_min, tnow.tm_sec);
}

int random_sleep()
{
	srand(time(NULL));
	int random = (rand() % (MAX_SLEEP_TIME - MIN_SLEEP_TIME + 1)) + MIN_SLEEP_TIME;
	return random;


}


DWORD WINAPI vesselRoute(void* param)
{
	random = random_sleep();
	int vesselId = *(int*)param;
	time_now(current_time);
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s Vessel no. %d has start sailing from Haifa port \n", current_time, vesselId);
	ReleaseMutex(printMutex);
	Sleep(random);



	/*haifa port now wants to write to the pipe to eilat */
	time_now(current_time);

	if (!WriteFile(writeToEilat, &vesselId, sizeof(int), &written, NULL))
	{
		WaitForSingleObject(printMutex, INFINITE);
		fprintf(stderr, "Error sending vessel to Eilat!\n");
		ReleaseMutex(printMutex);
		return;
	}
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s vessel no. %d Entered cannal ==> Eilat port\n", current_time, vesselId);
	ReleaseMutex(printMutex);
	Sleep(random);
	
	WaitForSingleObject(vesselMutexes[vesselId - 1], INFINITE);
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s vessel no. %d has arrived at Haifa port\n", current_time, vesselId);
	ReleaseMutex(printMutex);
	random = random_sleep();
	Sleep(random);
	 
	time_now(current_time);
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s vessel no. %d End sailing\n", current_time, vesselId);
	ReleaseMutex(printMutex);
	
}
