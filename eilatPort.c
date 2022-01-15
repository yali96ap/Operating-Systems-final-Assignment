#include "eilat.h"
#define MIN_SLEEP_TIME 500
#define MAX_SLEEP_TIME 3000
int counter = 0;
int num_of_cranes;
int freeCranes;
//HANDLE craneMutex;
HANDLE barrierMutex;
//HANDLE vesselMutex;
HANDLE printMutex;
HANDLE cranesThread[50];
int waiting = 0;
HANDLE vesselSemaphores[50];
HANDLE craneSemaphores[50];
HANDLE unloadSemaphores[50];
HANDLE vesselsThread[50];
int numOfvessel;
int vesselID[50];
int zevel;
char current_time[50];
int random;
int craneID[50];
int craneToVessel[50];
int vesselToCrane[50];
int craneWeight[50];
HANDLE ReadHandle, WriteHandle;
CHAR buffer[50];
DWORD read, written;
int vesselQueue[50];
int head = 0;
int tail = 0;

void main(int argc, const char* argv[])
{
	int  min_cranes = 1, max_cranes;
	srand(time(NULL));
	int converted;
	CHAR buffer[50];
	BOOL success;

	barrierMutex = CreateMutex(NULL, false, NULL);
	printMutex = CreateMutex(NULL, false, NULL);

	ReadHandle = GetStdHandle(STD_INPUT_HANDLE);
	WriteHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	// now have the child read from the pipe
	success = ReadFile(ReadHandle, &numOfvessel, sizeof(int), &read, NULL);

	// we have to output to stderr as stdout is redirected to the pipe
	now(current_time);
	if (success)
	{
		WaitForSingleObject(printMutex, INFINITE);
		fprintf(stderr, "%s Haifa asks for Eilat's approval to send %d vessels\n", current_time, numOfvessel);
		ReleaseMutex(printMutex);
	}
	else {
		fprintf(stderr, "Eilat: error reading from Haifa pipe\n");
		return;
	}
	// check if num of vessels is not prime
	bool notPrimeVessels = !isPrime(numOfvessel);


	// now write which vessel is returning to Haifa port
	if (!WriteFile(WriteHandle, &notPrimeVessels, sizeof(bool), &written, NULL))
	{
		fprintf(stderr, "Error writing to Haifa!\n");
		return;

	}


	if (!notPrimeVessels)
		return;

	for (int i = 1; i <= numOfvessel; i++)
	{
		vesselID[i - 1] = i;
	}

	num_of_cranes = cranes_num(numOfvessel); //randomize the number of cranes
	counter = numOfvessel;
	
	for (int i = 0; i < numOfvessel; i++)
	{
		vesselSemaphores[i] = CreateSemaphore(0, 0, 1, NULL);
		unloadSemaphores[i] = CreateSemaphore(0, 0, 1, NULL);
	}


	freeCranes = num_of_cranes;
	int cranesThread[50];
	for (size_t i = 1; i <= num_of_cranes; i++)
	{
		craneID[i - 1] = i;
		cranesThread[i - 1] = CreateThread(NULL, 0, craneThread, &craneID[i - 1], 0, &converted);

	}
	for (int i = 0; i < num_of_cranes; i++)
	{
		craneSemaphores[i] = CreateSemaphore(0, 0, 1, NULL);
	}

	//getting all vessels ID from haifa port through the pipe
	for (int i = 0; i < numOfvessel; i++)
	{
		if (!ReadFile(ReadHandle, &vesselID[i], sizeof(int), &read, NULL))
		{
			WaitForSingleObject(printMutex, INFINITE);
			fprintf(stderr, "Eilat: error reading from haifa pipe\n");
			ReleaseMutex(printMutex);
			return;
		}
		vesselsThread[i] = CreateThread(NULL, 0, vesselThread, &vesselID[i], 0, &converted);
		now(current_time);

	}

	WaitForMultipleObjects(numOfvessel, vesselsThread, true, INFINITE);
	now(current_time);
	fprintf(stderr, "%s Eilat: all vessels finished!\n", current_time);

	for (int i = 0; i < num_of_cranes; i++)
	{
		ReleaseSemaphore(craneSemaphores[i], 1, &zevel);
	}
	WaitForMultipleObjects(num_of_cranes, cranesThread, true, INFINITE);
	now(current_time);
	fprintf(stderr, "%s Eilat: all cranes threads finished!\n", current_time);

	CloseHandle(printMutex);
	CloseHandle(barrierMutex);
	for (int i = 0; i < numOfvessel; i++)
	{
		CloseHandle(vesselsThread[i]);
	}
	for (int i = 0; i < num_of_cranes; i++)
	{
		CloseHandle(cranesThread[i]);

	}
	CloseHandle(read);
	CloseHandle(written);
	now(current_time);
	fprintf(stderr, "%s Eilat port: process finished.\n", current_time);
}

int sleep_random()
{
	srand(time(NULL));
	int random = (rand() % (MAX_SLEEP_TIME - MIN_SLEEP_TIME + 1)) + MIN_SLEEP_TIME;
	return random;

}
int cranes_num(int vessels)
{
	while (true)
	{
		srand(time(NULL));
		int random = (rand() % (vessels - 2)) + 2;
		if (vessels % random == 0)
			return random;
	}
}


DWORD WINAPI craneThread(void* param)
{
	int craneID = *(int*)param;
	WaitForSingleObject(printMutex, INFINITE);
	now(current_time);
	fprintf(stderr, "%s Crane %d is created!\n", current_time, craneID);
	ReleaseMutex(printMutex);
	while (true)
	{
		if (counter == 0)
		{
			now(current_time);
			WaitForSingleObject(printMutex, INFINITE);
			fprintf(stderr, "%s Crane no. %d is finished!\n", current_time, craneID);
			ReleaseMutex(printMutex);
			break;
		}

		now(current_time);
		random = sleep_random();
		//Crane waiting to be released!
		WaitForSingleObject(craneSemaphores[craneID - 1], INFINITE);

		int vesselId = vesselToCrane[craneID - 1];
		
		Sleep(random);
		
		WaitForSingleObject(printMutex, INFINITE);
		fprintf(stderr, "%s Crane no. %d is unloading %d tons from vessel no. %d \n", current_time, craneID, craneWeight[craneID - 1], vesselToCrane[(craneID - 1)] + 1);
		ReleaseMutex(printMutex);
		ReleaseSemaphore(unloadSemaphores[vesselId - 1], 1, &zevel);

	}
}
int weight;
DWORD WINAPI vesselThread(void* param)
{
	random = sleep_random();
	now(current_time);

	int vesselID = *(int*)param;
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s Vessel no. %d has Entered Eilat port\n", current_time, vesselID);
	ReleaseMutex(printMutex);

	now(current_time);
	Sleep(random);
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s Vessel no. %d has Entered the Barrier\n", current_time, vesselID);
	ReleaseMutex(printMutex);
	enterUnloadingQuay(vesselID);

	random = sleep_random();
	now(current_time);
	Sleep(random);

	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s Vessel no. %d is sailing to the unloading quay\n", current_time, vesselID);
	ReleaseMutex(printMutex);

	random = sleep_random();
	Sleep(random);
	now(current_time);

	srand(time(NULL) + vesselID + 543);
	weight = rand() % (50 - 5 + 1) + 5;

	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s Vessel no. %d has %d tons to unload \n", current_time, vesselID, weight);
	ReleaseMutex(printMutex);

	int craneId = craneToVessel[vesselID - 1];
	craneWeight[craneId - 1] = weight;

	now(current_time);
	WaitForSingleObject(printMutex, INFINITE);
	fprintf(stderr, "%s Vessel no. %d is served by crane no. %d \n", current_time, vesselID, craneToVessel[vesselID - 1]);
	ReleaseMutex(printMutex);


	vesselToCrane[craneId - 1] = vesselID;

	ReleaseSemaphore(craneSemaphores[craneId - 1], 1, &zevel);
	WaitForSingleObject(vesselSemaphores[vesselID - 1], INFINITE);
	exitUnloadingQuay();

	random = sleep_random();
	Sleep(random);

	now(current_time);
	
	WaitForSingleObject(printMutex, INFINITE);
	
	fprintf(stderr, "%s Vessel no. %d is exiting the unloading quay. \n", current_time, vesselID);
	
	ReleaseMutex(printMutex);
	
	random = sleep_random();
	Sleep(random);
	now(current_time);
	
	WaitForSingleObject(printMutex, INFINITE);
	
	fprintf(stderr, "%s vessel no. %d Entered cannal ==> Haifa port\n", current_time, vesselID);
	
	ReleaseMutex(printMutex);
	counter--;

	// Now write which vessel is returning to Haifa port
	if (!WriteFile(WriteHandle, &vesselID, sizeof(int), &written, NULL))
	{
		fprintf(stderr, "Error writing to Haifa!\n");
		return;

	}
}


void enterUnloadingQuay(int vesselID)
{
	WaitForSingleObject(barrierMutex, INFINITE);
	waiting++;
	vesselQueue[tail] = vesselID;
	tail++;
	barrierCheck();
	ReleaseMutex(barrierMutex);
	WaitForSingleObject(vesselSemaphores[vesselID - 1], INFINITE);
	ReleaseSemaphore(vesselSemaphores[vesselID - 1], 1, &zevel);
}


void exitUnloadingQuay()
{
	WaitForSingleObject(barrierMutex, INFINITE);
	freeCranes++;
	barrierCheck();
	ReleaseMutex(barrierMutex);

}

void barrierCheck()
{
	int dequeuedVessel;

	if (freeCranes == num_of_cranes && waiting >= num_of_cranes)
	{
		for (int i = 0; i < num_of_cranes; i++)
		{
			freeCranes--;
			dequeuedVessel = vesselQueue[head];
			ReleaseSemaphore(vesselSemaphores[dequeuedVessel - 1], 1, &zevel);
			head++;
			waiting--;
			craneToVessel[dequeuedVessel - 1] = i + 1;
		}


	}
}

void now(char* time_now)
{
	time_t now = time(NULL);
	struct tm tnow;
	localtime_s(&tnow, &now);
	sprintf(time_now, "[%02d:%02d:%02d]", tnow.tm_hour, tnow.tm_min, tnow.tm_sec);
}



bool isPrime(int num)
{

	int i, flag = 0;

	for (i = 2; i <= num / 2; ++i) {

		// condition for non-prime
		if (num % i == 0) {
			flag = 1;
			break;
		}
	}

	if (num == 1) {
		return false;

	}
	else {
		if (flag == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return false;
}