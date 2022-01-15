#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>    
#define MIN_SLEEP_TIME 500
#define MAX_SLEEP_TIME 3000
DWORD WINAPI vesselRoute(void* param);
void time_now(char* clock_time);
int random_sleep();


