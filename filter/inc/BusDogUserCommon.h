#ifndef __BUSDOG_USER_COMMON_H__
#define __BUSDOG_USER_COMMON_H__

//
// The following value is arbitrarily chosen from the space defined by Microsoft
// as being "for non-Microsoft use"
//
#define FILE_DEVICE_BUSDOG 0x0F59

//
// Device control codes - values between 2048 and 4095 arbitrarily chosen
//
#define IOCTL_BUSDOG_GET_BUFFER CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2049,               \
                                         METHOD_OUT_DIRECT,  \
                                         FILE_READ_ACCESS)   \

#define IOCTL_BUSDOG_START_FILTERING CTL_CODE(FILE_DEVICE_BUSDOG,\
                                          2050,              \
                                          METHOD_BUFFERED,   \
                                          FILE_READ_ACCESS)  \

#define IOCTL_BUSDOG_STOP_FILTERING CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2051,               \
                                         METHOD_BUFFERED,    \
                                         FILE_READ_ACCESS)   \


#define IOCTL_BUSDOG_PRINT_DEVICES CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2052,               \
                                         METHOD_BUFFERED,    \
                                         FILE_READ_ACCESS)   \

#define IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2053,               \
                                         METHOD_BUFFERED,    \
                                         FILE_WRITE_ACCESS)  \

#define IOCTL_BUSDOG_GET_DEVICE_LIST CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2054,               \
                                         METHOD_BUFFERED,    \
                                         FILE_READ_ACCESS)   \

#define IOCTL_BUSDOG_GET_DEBUG_LEVEL CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2055,               \
                                         METHOD_BUFFERED,    \
                                         FILE_READ_ACCESS)   \

#define IOCTL_BUSDOG_SET_DEBUG_LEVEL CTL_CODE(FILE_DEVICE_BUSDOG,\
                                         2056,               \
                                         METHOD_BUFFERED,    \
                                         FILE_WRITE_ACCESS)  \

//
// struct definition for IOCTL_BUSDOG_SET_DEVICE_FILTER_ENABLED
//

typedef struct 
{
    DWORD DeviceId;
    BOOLEAN FilterEnabled;
} BUSDOG_FILTER_ENABLED, *PBUSDOG_FILTER_ENABLED;

//
// All of the following definitions are for use with 
//  IOCTL_BUSDOG_GET_BUFFER
// 

//
// Each entry in the request buffer structure is tagged
//  with a request type, which indicates the type of 
//  WDFREQUEST it represents
//
typedef enum _REQUEST_TYPE {

    BusDogReadRequest = 0x2000,
    BusDogWriteRequest,
    BusDogDeviceControlRequest,
    BusDogInternalDeviceControlRequest,
    BusDogPnPRequest,
    BusDogMaxRequestType

} BUSDOG_REQUEST_TYPE;

typedef struct
{
    // Seconds
    LONG sec; 

    // Microseconds
    LONG usec;
} BUSDOG_TIMESTAMP;

//
// This is where we store each individual trace
//
typedef struct
{

    DWORD DeviceId;

    BUSDOG_REQUEST_TYPE Type;

    BUSDOG_TIMESTAMP Timestamp;

    size_t BufferSize;

} BUSDOG_FILTER_TRACE, *PBUSDOG_FILTER_TRACE; // Data buffer follows this structure in memory

//
// struct definition for IOCTL_BUSDOG_GET_DEVICE_LIST
//

typedef struct
{

    DWORD DeviceId;

    BOOLEAN Enabled;

    size_t PhysicalDeviceObjectNameSize;

} BUSDOG_DEVICE_ID, *PBUSDOG_DEVICE_ID; // unicode string follows this structure in memory

//
// struct definition for IOCTL_BUSDOG_GET_DEBUG_LEVEL/IOCTL_BUSDOG_SET_DEBUG_LEVEL
//

typedef struct
{

    DWORD DebugLevel;

} BUSDOG_DEBUG_LEVEL, *PBUSDOG_DEBUG_LEVEL;

#define BUSDOG_DEBUG_INFO  3
#define BUSDOG_DEBUG_WARN  2
#define BUSDOG_DEBUG_ERROR 1
#define BUSDOG_DEBUG_NONE  0

#endif

