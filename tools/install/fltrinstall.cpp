///////////////////////////////////////////////////////////////////////////////
//
//    (C) Copyright 1995 - 2006 OSR Open Systems Resources, Inc.
//    All Rights Reserved
//
//    This sofware is supplied for instructional purposes only.
//
//    OSR Open Systems Resources, Inc. (OSR) expressly disclaims any warranty
//    for this software.  THIS SOFTWARE IS PROVIDED  "AS IS" WITHOUT WARRANTY
//    OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION,
//    THE IMPLIED WARRANTIES OF MECHANTABILITY OR FITNESS FOR A PARTICULAR
//    PURPOSE.  THE ENTIRE RISK ARISING FROM THE USE OF THIS SOFTWARE REMAINS
//    WITH YOU.  OSR's entire liability and your exclusive remedy shall not
//    exceed the price paid for this material.  In no event shall OSR or its
//    suppliers be liable for any damages whatsoever (including, without
//    limitation, damages for loss of business profit, business interruption,
//    loss of business information, or any other pecuniary loss) arising out
//    of the use or inability to use this software, even if OSR has been
//    advised of the possibility of such damages.  Because some states/
//    jurisdictions do not allow the exclusion or limitation of liability for
//    consequential or incidental damages, the above limitation may not apply
//    to you.
//
//    OSR Open Systems Resources, Inc.
//    105 Route 101A Suite 19
//    Amherst, NH 03031  (603) 595-6500 FAX: (603) 595-6503
//    email bugs to: bugs@osr.com
//
//
//    MODULE:
//
//      fltrinstall.cpp
//
//    ABSTRACT:
//
//      This file contains *MOST* of the user mode code necessary to 
//      install a WDF filter driver. The user is still required to put
//      the Upper/LowerFilters key in the appropriate place, all this 
//      code does is install the service.
//
//      Based heavily on the techniques used in the installer for the 
//      non-PnP driver in the WDF distribution
//
//    AUTHOR(S):
//
//      OSR Open Systems Resources, Inc.
// 
//    REVISION:   
//
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <wdfinstaller.h>

/////////////
// GLOBALS //
/////////////
PWCHAR    ServiceName;
PWCHAR    SYSFile;
BOOLEAN   KMDF19;
BOOLEAN   IsDemand;
BOOLEAN   IsInstall;
HMODULE   WdfCoinstaller;
SC_HANDLE SCManager;
WCHAR     INFFileWithPath[255];
WCHAR     INFSectionName[255];


///////////////////////////
// WDF COINSTALLER FUNCS //
///////////////////////////
PFN_WDFPREDEVICEINSTALL  WdfPreDeviceInstallPtr;
PFN_WDFPOSTDEVICEINSTALL WdfPostDeviceInstallPtr;
PFN_WDFPREDEVICEREMOVE   WdfPreDeviceRemovePtr;
PFN_WDFPOSTDEVICEREMOVE  WdfPostDeviceRemovePtr;


//////////////////////////
// FORWARD DECLARATIONS //
//////////////////////////
BOOLEAN
ParseParameters(
    int argc, 
    WCHAR **argv
    );

BOOLEAN
InitializeWdfCoinstaller(
    VOID
    );

BOOLEAN
GenerateINFFile(
    VOID
    );

VOID
DoInstallation(
    VOID
    );

VOID
DoRemove(
    VOID
    );



///////////////
// CONSTANTS //
///////////////
#define USAGE_STRING "Usage: wdffltrinstall {-i|-r} servicename "  \
                     "{-d|-b} driver.sys {-kmdf17|-kmdf19}\n\n"    \
                     "  {-i|-r} -- Indicates whether this is an "  \
                     "installation or a removal\n"                 \
                     "  {-d|-b} -- Indicates whether this service "\
                     "should be installed as demand start or boot "\
                     "start\n"                                     \
                     "  {-kmdf17|-kmdf19} -- Indicates whether "   \
                     "this service should uses kmdf 1.7 "          \
                     "coinstaller or kmdf 1.9\n"


#define INSTALL_OR_REMOVE_OPT   1
#define SERVICE_NAME_OPT        2
#define DEMAND_OR_BOOT_OPT      3
#define SYS_FILE_OPT            4
#define KMDF_VERSION_OPT        5
#define ARGUMENT_COUNT          6


///////////////////////////////////////////////////////////////////////////////
//
//  wmain
//
//      Entry point for the application
//
//  INPUTS:
//
//      argc, argv...You know...See ParseParameters and USAGE_STRING for more
//      info
//
//  OUTPUTS:
//
//      None
//
//  RETURNS:
//
//      None.
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
void __cdecl wmain(int argc, WCHAR **argv) {

    //
    // Do all of the necessary initialization
    //
    if (!ParseParameters(argc, argv) ||
        !InitializeWdfCoinstaller()  ||
        !GenerateINFFile()) {
        //
        // Whoever failed printed out an appropriate error,
        //  just return.
        //
        return;
    }

    //
    // Open the SCM...
    //
    SCManager = OpenSCManager(NULL,
                              NULL,
                              SC_MANAGER_ALL_ACCESS);

    if (!SCManager) {
        printf("Couldn't open SCM: GLE - %d\n", GetLastError());
        return;
    }

    //
    // Let the user know what she's doing...
    //
    printf("You want to %s %ls, which is %s start and uses %ls\n",
        IsInstall ? "INSTALL" : "REMOVE", 
        ServiceName,
        IsDemand ? "DEMAND" : "BOOT",
        SYSFile);

    //
    // Take the appropriate action...
    //
    if (IsInstall) {
        DoInstallation();
    } else {
        DoRemove();
    }
    
    //
    // Delete the INF we created...
    //
    printf("Deleting %ls...\n", INFFileWithPath);
    DeleteFile(INFFileWithPath);
    return;

}

///////////////////////////////////////////////////////////////////////////////
//
//  ParseParameters
//
//      Subroutine to deal with parsing all of the parameters.
//
//  INPUTS:
//
//      argc, argv...You know...
//
//  OUTPUTS:
//
//      None, does fill in some globals as a result, though.
//
//  RETURNS:
//
//      TRUE if successful, FALSE otherwise
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN
ParseParameters(
    int argc, 
    WCHAR **argv
    ) {

    if (argc != ARGUMENT_COUNT) {
        printf(USAGE_STRING);
        return FALSE;

    }

    //
    // Install or remove?
    //
    if (_wcsicmp(argv[INSTALL_OR_REMOVE_OPT], L"-i") == 0) {
        IsInstall = TRUE;
    } else if (_wcsicmp(argv[INSTALL_OR_REMOVE_OPT], L"-r") == 0) {
        IsInstall = FALSE;
    } else {
        printf("Unknown option %ls\n", argv[1]);
        return FALSE;
    }


    //
    // Service name...
    //
    ServiceName = argv[SERVICE_NAME_OPT];


    //
    // Demand or boot start...
    //
    if (_wcsicmp(argv[DEMAND_OR_BOOT_OPT], L"-d") == 0) {
        IsDemand = TRUE;
    } else if (_wcsicmp(argv[DEMAND_OR_BOOT_OPT], L"-b") == 0) {
        IsDemand = FALSE;
    } else {
        printf("Unknown option %ls\n", argv[DEMAND_OR_BOOT_OPT]);
        return FALSE;
    }


    //
    // And the SYS...
    // 
    SYSFile = argv[SYS_FILE_OPT];


    //
    // KMDF version
    //
    KMDF19 = _wcsicmp(argv[KMDF_VERSION_OPT], L"-kmdf19") == 0;


    return TRUE;

}



///////////////////////////////////////////////////////////////////////////////
//
//  InitializeWdfCoinstaller
//
//      Helper routine to load the coinstaller DLL and get pointers to the 
//      routines that we need to call during install/remove
//
//  INPUTS:
//
//      None.
//
//  OUTPUTS:
//
//      None, does fill in some globals as a result, though.
//
//  RETURNS:
//
//      TRUE if successful, FALSE otherwise
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN
InitializeWdfCoinstaller(
    VOID
    ) {

    WCHAR currentDir[512];
    WCHAR coinstallerPath[512+sizeof("\\WdfCoInstaller0100X.dll")];

    //
    // Only look in the current directory. Could be extended
    //  to look elsewhere, but this makes life easier.
    //
    if (!GetCurrentDirectory(512, currentDir)) {
        printf("Couldn't get current directory - GLE %d\n", GetLastError());
        return FALSE;
    }

    if (KMDF19) {
        swprintf(coinstallerPath, L"%ls\\WdfCoInstaller01009.dll", currentDir);
    }
    else {
        swprintf(coinstallerPath, L"%ls\\WdfCoInstaller01007.dll", currentDir);
    }

    printf("Loading %ls...\n", coinstallerPath);

    WdfCoinstaller = LoadLibrary(coinstallerPath);

    if (!WdfCoinstaller) {
        printf("Couldn't load %ls - GLE %d\n", coinstallerPath, GetLastError());
        return FALSE;

    }

    WdfPreDeviceInstallPtr = (PFN_WDFPREDEVICEINSTALL)
                                    GetProcAddress(WdfCoinstaller,
                                                   "WdfPreDeviceInstall");
    WdfPostDeviceInstallPtr = (PFN_WDFPOSTDEVICEINSTALL)
                                    GetProcAddress(WdfCoinstaller,
                                                   "WdfPostDeviceInstall");
    WdfPreDeviceRemovePtr = (PFN_WDFPREDEVICEREMOVE)
                                    GetProcAddress(WdfCoinstaller,
                                                   "WdfPreDeviceRemove");
    WdfPostDeviceRemovePtr = (PFN_WDFPOSTDEVICEREMOVE)
                                    GetProcAddress(WdfCoinstaller,
                                                   "WdfPostDeviceRemove");

    if (!WdfPreDeviceInstallPtr    ||
        !WdfPostDeviceInstallPtr   ||
        !WdfPreDeviceRemovePtr     ||
        !WdfPostDeviceRemovePtr) {

        printf("One of the GetProcAddress calls failed..."\
               "PrI: %p, PoI: %p, PrR: %p, PoR: %p\n",
                WdfPreDeviceInstallPtr,
                WdfPostDeviceInstallPtr,
                WdfPreDeviceRemovePtr,
                WdfPostDeviceRemovePtr);
        FreeLibrary(WdfCoinstaller);
        return FALSE;

    }

    printf("Found all of the necessary entry points in the coinstaller\n");

    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//
//  GenerateINFFile
//
//      You need a dummy INF file in order to call the WDF coinstaller. In 
//      order to avoid confusion, we'll generate our own  on the fly, pass 
//      it to the coinstaller, then delete it on the way out. 
//
//  INPUTS:
//
//      None.
//
//  OUTPUTS:
//
//      None, does fill in some globals as a result, though.
//
//  RETURNS:
//
//      TRUE if successful, FALSE otherwise
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN
GenerateINFFile(
    VOID
    ) {

    HANDLE  file;
    HANDLE  mappedFile = NULL;
    DWORD   bytesWritten;
    BOOLEAN ret = TRUE;
    PCHAR   mappedBuffer = NULL;

    //
    // Create the INF in the current directory.
    //
    swprintf(INFFileWithPath, L".\\%ls_temp.inf", ServiceName);

    //
    // We need to name the section of the INF
    //  that we'll be using as our WDF section.
    //  ServiceName.NT.Wdf sounds good.
    //
    swprintf(INFSectionName, L"%ls.NT.Wdf", ServiceName);

    printf("Creating %ls...**If you have an INF, it is being IGNORED!!**\n",
        INFFileWithPath);

    file = CreateFile(INFFileWithPath,
                      GENERIC_ALL,
                      0,
                      NULL, 
                      CREATE_ALWAYS,
                      0,
                      NULL);

    if (file == INVALID_HANDLE_VALUE) {
        printf("Couldn't create dummy INF: GLE - %d\n", GetLastError());
        ret = FALSE;
        goto cleanup;
    }

    //
    // For no good reason we'll fill in the file contents
    //  by using a memory mapping....
    //

    //
    // Make the file "big enough". 4K should more than do, disk
    //  space is cheap
    //
    mappedFile = CreateFileMapping(file,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   0x1000,
                                   NULL);

    if (!mappedFile) {
        printf("Couldn't create mapping dummy INF: GLE - %d\n", GetLastError());
        ret = FALSE;
        goto cleanup;
    }


    //
    // Get a buffer for this file...
    //
    mappedBuffer = (PCHAR)MapViewOfFile(mappedFile,
                                        FILE_MAP_ALL_ACCESS,
                                        0,
                                        0,
                                        0x1000);

    if (!mappedBuffer) {
        printf("Couldn't map dummy INF: GLE - %d\n", GetLastError());
        ret = FALSE;
        goto cleanup;
    }


    //
    // And fill in the contents by sprintf-ing the appropriate
    //  pieces together. The file only has to contain:
    //
    //  [Version]
    //  signature = "$Windows NT$"
    //
    //  [ServiceName.NT.Wdf]
    //  KmdfService = ServiceName, wdfsect
    //  [wdfsect]
    //  KmdfLibraryVersion = 1.0
    //
    sprintf(mappedBuffer, 
            "[Version]\r\n"                   \
            "signature = \"$Windows NT$\"\r\n"\
            "\r\n"                            \
            "[%ls.NT.Wdf]\r\n"                \
            "KmdfService = %ls, wdfsect\r\n"  \
            "[wdfsect]\r\n"                   \
            "KmdfLibraryVersion = 1.0\r\n",
            ServiceName,
            ServiceName);

   
cleanup:

    if (mappedBuffer) {
        if (!UnmapViewOfFile(mappedBuffer)) {
            printf("Couldn't unmap dummy INF: GLE - %d\n", GetLastError());
        }
    }

    if (mappedFile) {
        CloseHandle(mappedFile);
    }

    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
    }

    return ret;

}


///////////////////////////////////////////////////////////////////////////////
//
//  DoInstallation
//
//      Routine to install the service while making the appropriate calls to 
//      the coinstaller.
//
//  INPUTS:
//
//      None.
//
//  OUTPUTS:
//
//      None.
//
//  RETURNS:
//
//      None.
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
VOID
DoInstallation(
    VOID
    ) {
   
    SC_HANDLE  schService;
    WCHAR      windowsDir[512];
    WCHAR      fullDriverPath[512];

    //
    // Because we're doing the entire installation ourselves,
    //  copy the file to system32\drivers
    //
    if (!GetWindowsDirectory(windowsDir, 512)) {
        printf("Couldn't get windows dir - GLE: %d\n", GetLastError());
        return;
    }

    swprintf(fullDriverPath, L"%ls\\system32\\drivers\\%ls",
        windowsDir, SYSFile);

    printf("Copying %ls to %ls\n", SYSFile, fullDriverPath);

    if (!CopyFile(SYSFile, fullDriverPath, FALSE)) {
        printf("Couldn't copy file - GLE: %d\n", GetLastError());
        return;
    }

    //
    // Call the coinstaller for pre-install
    //
    printf("Calling the WDF coinstaller for pre-install\n"\
           "  (Don't panic, this can take a while)\n");
    if (WdfPreDeviceInstallPtr(INFFileWithPath, INFSectionName)) {

        printf("Pre install failed - GLE: %d\n", GetLastError());
        return;

    }

    //
    // Create the service entry for the driver
    //
    printf("Creating the service\n");
    schService = CreateService (SCManager,
                                ServiceName,
                                ServiceName,
                                SERVICE_ALL_ACCESS,
                                SERVICE_KERNEL_DRIVER,
                                IsDemand ? 
                                  SERVICE_DEMAND_START : 
                                       SERVICE_BOOT_START,
                                SERVICE_ERROR_NORMAL,
                                fullDriverPath,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

    if (!schService) {

        printf("Couldn't create service - GLE: %d\n", GetLastError());
        return;

    }

    CloseServiceHandle (schService);

    printf("Calling the WDF coinstaller for post install\n");
    if (WdfPostDeviceInstallPtr(INFFileWithPath, INFSectionName)) {

        printf("Post install failed - GLE: %d\n", GetLastError());
        return;

    }

    printf("Congrats! Install is complete\n\n");


    printf("****************************************************\n"\
           "* NOTE:                                            *\n"\
           "* This installer does NOT bother to register the   *\n"\
           "* driver as a filter! You must manually create the *\n"\
           "* Upper/LowerFilters key yourself!                 *\n"\
           "****************************************************\n\n");
    return;
}


///////////////////////////////////////////////////////////////////////////////
//
//  DoRemove
//
//      Routine to remove the service while making the appropriate calls to 
//      the coinstaller.
//
//  INPUTS:
//
//      None.
//
//  OUTPUTS:
//
//      None.
//
//  RETURNS:
//
//      None.
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
VOID
DoRemove(
    VOID
    ) {
    
    SC_HANDLE schService;

    printf("Calling the WDF coinstaller for pre remove\n");
    if (WdfPreDeviceRemovePtr(INFFileWithPath, INFSectionName) != ERROR_SUCCESS) {

        printf("Pre remove failed - GLE: %d\n", GetLastError());

    }

    //
    // Open the service...
    //
    printf("Opening the %ls service\n", ServiceName);
    schService = OpenService(SCManager,
                             ServiceName,
                             SERVICE_ALL_ACCESS);

    if (!schService) {
        printf("Couldn't open service - GLE: %d\n", GetLastError());
        return;

    }

    //
    // Delete the service...
    //
    printf("Deleting the %ls service\n", ServiceName);
    if (!DeleteService(schService)) {
        printf("Couldn't delete service - GLE: %d\n", GetLastError());
    }

    CloseServiceHandle(schService);

    printf("Calling the WDF coinstaller for post remove\n");
    if (WdfPostDeviceRemovePtr(INFFileWithPath, INFSectionName) != ERROR_SUCCESS) {

        printf("Post remove failed - GLE: %d\n", GetLastError());
        return;

    }

    printf("Congrats! Remove is complete\n");
    return;
}
