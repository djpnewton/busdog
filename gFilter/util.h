#include <stdio.h>

#ifndef _UTIL_H_
#define _UTIL_H_
	
/*	#define TRUE 	1
	#define FALSE	0*/

	#define STR(x) #x
	#define XSTR(x) STR(x)

	#define LOG(level, fmt, ...)	printf("LOG: "__FILE__ ": " XSTR(__LINE__) ": " fmt, __VA_ARGS__)
	#define DBG(level, fmt, ...)	printf("DBG: "__FILE__ ": " XSTR(__LINE__) ": " fmt, __VA_ARGS__)
	#define ERR(level, fmt, ...)	printf("ERR: "__FILE__ ": " XSTR(__LINE__) ": " fmt, __VA_ARGS__)

#endif
