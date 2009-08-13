// Assumes you have defined DRIVERNAME before including this

#if !defined(_LOG_H_)
#define _LOG_H_

//
// Debugging Macros
//
#define STR(x) #x
#define XSTR(x) STR(x)

#ifdef DEBUG	
	#define DEBUG(s, ...) 	DbgPrint(DRIVERNAME ": DBG: " XSTR(__LINE__) ": " s, __VA_ARGS__)
	#define INFO(s, ...)	DbgPrint(DRIVERNAME ": INF: " XSTR(__LINE__) ": " s, __VA_ARGS__)
	#define WARN(s, ...)	DbgPrint(DRIVERNAME ": WRN: " XSTR(__LINE__) ": " s, __VA_ARGS__)
	#define ERROR(s, ...)	DbgPrint(DRIVERNAME ": ERR: " XSTR(__LINE__) ": " s, __VA_ARGS__)
#else
	#define DEBUG(s, ...) 	DbgPrint(DRIVERNAME ": DBG: " XSTR(__LINE__) ": " s, __VA_ARGS__) // For now the non dbg target can contain all
	#define INFO(s, ...)	DbgPrint(DRIVERNAME ": INF: " XSTR(__LINE__) ": " s, __VA_ARGS__)
	#define WARN(s, ...)	DbgPrint(DRIVERNAME ": WRN: " XSTR(__LINE__) ": " s, __VA_ARGS__)
	#define ERROR(s, ...)	DbgPrint(DRIVERNAME ": ERR: " XSTR(__LINE__) ": " s, __VA_ARGS__)
#endif	

#endif 
