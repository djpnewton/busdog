#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")

#include "service.h"

int CreateSvc(TCHAR* SvcName, TCHAR* SvcPath, bool IsDemand)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus; 

    int res = 0;

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        res = 1;
        goto cleanup;
    }

    // Create the service

     schService = CreateService(
        schSCManager,       // SCM database 
        SvcName,            // name of service 
        SvcName,
        SC_MANAGER_CREATE_SERVICE,
        SERVICE_KERNEL_DRIVER,
        IsDemand ? SERVICE_DEMAND_START : SERVICE_BOOT_START,
        SERVICE_ERROR_NORMAL,
        SvcPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);
 
    if (schService == NULL)
    { 
        printf("CreateService \"%s\" failed (%d)\n", SvcName, GetLastError()); 
        CloseServiceHandle(schSCManager);
        res = 2;
        goto cleanup;
    }
    else
        printf("Service created successfully\n");

cleanup:
 
    if (schService != NULL)
        CloseServiceHandle(schService); 

    if (schSCManager != NULL)
        CloseServiceHandle(schSCManager);

    return res;
}

int DeleteSvc(TCHAR* SvcName)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus; 

    int res = 0;

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        res = 1;
        goto cleanup;
    }

    // Get a handle to the service.

    schService = OpenService( 
        schSCManager,       // SCM database 
        SvcName,            // name of service 
        DELETE);            // need delete access 
 
    if (schService == NULL)
    { 
        printf("OpenService \"%s\" failed (%d)\n", SvcName, GetLastError()); 
        CloseServiceHandle(schSCManager);
        res = 2;
        goto cleanup;
    }

    // Delete the service.
 
    if (! DeleteService(schService) ) 
    {
        printf("DeleteService failed (%d)\n", GetLastError()); 
        res = 3;
    }
    else 
        printf("Service deleted successfully\n"); 

cleanup:
 
    if (schService != NULL)
        CloseServiceHandle(schService); 

    if (schSCManager != NULL)
        CloseServiceHandle(schSCManager);

    return res;
}
