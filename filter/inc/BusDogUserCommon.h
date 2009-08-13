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


typedef struct 
{
    DWORD DeviceId;
    BOOLEAN FilterEnabled;
} BUSDOG_FILTER_ENABLED, *PBUSDOG_FILTER_ENABLED;

#endif

