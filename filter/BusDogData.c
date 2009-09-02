#include "BusDogCommon.h"

//
// BusDogFiltering controls whether to log filter traces or not
//
BOOLEAN         BusDogFiltering = FALSE;

//
// BusDogDebugLevel controls debug output
//
DWORD           BusDogDebugLevel = BUSDOG_DEBUG_WARN;


//
// Collection object is used to store all the FilterDevice objects so
// that any event callback routine can easily walk thru the list and pick a
// specific instance of the device for filtering.
//
WDFCOLLECTION   BusDogDeviceCollection;
WDFWAITLOCK     BusDogDeviceCollectionLock;

