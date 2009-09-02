#if !defined(_BUSDOG_DATA_H_)
#define _BUSDOG_DATA_H_

extern BOOLEAN         BusDogFiltering;
extern DWORD           BusDogDebugLevel;

extern WDFCOLLECTION   BusDogDeviceCollection;
extern WDFWAITLOCK     BusDogDeviceCollectionLock;

#endif

