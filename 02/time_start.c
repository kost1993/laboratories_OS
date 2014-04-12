#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static __inline__ unsigned long long rdtsc()
{
	unsigned long long x;
	__asm__ volatile(".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
int main()
{
	unsigned long long timer_count_1;
	unsigned long long timer_count_2;
	unsigned long long count_in_one_sec;
	timer_count_1 = rdtsc();
	sleep(1);
	timer_count_2 = rdtsc();
	count_in_one_sec = timer_count_2 - timer_count_1;
	printf("\n");
	printf("Counts in processor in one sec: %llu\n", count_in_one_sec);
	printf("Time from power PC: %llu sec\n", timer_count_1 / count_in_one_sec);
	return 0;
}
