#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <time.h>    
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Windows.h>
#define MIN_SLEEP_TIME 500
#define MAX_SLEEP_TIME 3000
bool isPrime(int num);
void now(char* time_now);
DWORD WINAPI craneThread(void* param);
DWORD WINAPI vesselThread(void* param);
void enterUnloadingQuay(int vesselID);
void exitUnloadingQuay();
void barrierCheck();
int cranes_num(int vessels);
int sleep_random();

