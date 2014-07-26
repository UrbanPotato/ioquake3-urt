// under GPL v2

// Hopefully decrease impact of the (frequently called) timers

#if !defined(WIN32)
	#include <stdio.h>
	#include <sys/time.h>
	extern time_t initial_tv_sec;
#else
	#include <Windows.h>
	extern LONGLONG initial_lpPerformanceCount;
	extern int initial_timeGetTime;
	extern int microdifference;
	unsigned long long Sys_Microseconds (void);
#endif

void Sys_InitTimers(void) {

	#if !defined(WIN32)
	
		struct timeval tp;

		gettimeofday(&tp, NULL);

		initial_tv_sec = tp.tv_sec;

	#else
		
		LARGE_INTEGER lpPerformanceCount, lpFrequency;

		QueryPerformanceCounter(&lpPerformanceCount);
		QueryPerformanceFrequency(&lpFrequency);

		// This has to be done first since Sys_Microseconds() is called right after:
		initial_lpPerformanceCount = lpPerformanceCount.QuadPart; 

		initial_timeGetTime = timeGetTime(); 

		microdifference = initial_timeGetTime - Sys_Microseconds() / 1000; // difference facilitates live switching

	#endif
	

}