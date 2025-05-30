/*
 * Tool name   : ListRunningServices
 * Description : A tool to list all running services on a system
 *               and to determine the its exact service type and run state.
 * Version     : 0.2
 * OS          : Tested on Microsoft Windows XP
 * Todo        : Stopping/starting depending services.
 *
 * Changes     : 2009.05.22 - Added stop/continue features.
 * 
 *
 */


#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>

#define MAX_BUF_SIZE 1024

#pragma comment(lib, "Advapi32.lib")


/*
 * Constants, data types and function forward declarations.
 *
 */

DWORD listRunningServices(void);
void help(char *pToolName);
DWORD startService(char *pServiceName);
DWORD stopService(char *pServiceName);


char gServiceStatus[9][MAX_BUF_SIZE + 1] = {"?", "stopped", "starting", "stoping", "running", "will continue", "pausing", "paused"};






/*
 * Program entry point
 *
 */

int main(int argc, char **argv)
{
  int lRetVal = 0;


  if (argc == 1)
  {
    listRunningServices();
  } else if (argc == 2 && !strcmp(argv[1], "-l")) {
    listRunningServices();
  } else if (argc == 2 && !strcmp(argv[1], "-h")) {
    help(argv[0]);
  } else if (argc == 3 && !strcmp(argv[1], "-c")) {
    startService(argv[2]);
  } else if (argc == 3 && !strcmp(argv[1], "-s")) {
    stopService(argv[2]);
  } else {
    help(argv[0]);
  }

  return(lRetVal);
}






/*
 * Enumerate all service regardless of its state.
 *
 */

DWORD listRunningServices(void)
{
  SC_HANDLE lSCHandle;
  SC_HANDLE lDetailSCHandle;
  char lTemp[MAX_BUF_SIZE + 1];
  int lRetVal = 0;
  int lFuncRetVal = 0;
  DWORD lResumeHandle = 0;
  DWORD lServicdType = SERVICE_WIN32|SERVICE_DRIVER;
  DWORD lSize = 0;
  DWORD lServicesReturned;
  DWORD lByteNeeded;
  LPENUM_SERVICE_STATUS_PROCESS lServices;
  LPQUERY_SERVICE_CONFIG lConfig;
  DWORD lCounter = 0;
  char lServiceType[MAX_BUF_SIZE + 1];


  if( !(lSCHandle = OpenSCManager(NULL,NULL,SC_MANAGER_ENUMERATE_SERVICE)))
  {
    lRetVal = 1;
	goto END;
  }

  if (! (lServices = (LPENUM_SERVICE_STATUS_PROCESS) LocalAlloc(LPTR, 65535)))
  {
    lRetVal = 2;
    goto END;
  }

  ZeroMemory(lServices, sizeof(lServices));

  if (EnumServicesStatusEx(lSCHandle, SC_ENUM_PROCESS_INFO, lServicdType, SERVICE_STATE_ALL,
     (LPBYTE) lServices, 65535, &lSize, &lServicesReturned, &lResumeHandle, NULL) == 0) 
  {
    lRetVal = 3;
	goto END;
  }


  /*
   * Determine all services, its type and print it out.
   *
   */

  for (lCounter = 0; lCounter < lServicesReturned; lCounter++) 
  {
	if ((lDetailSCHandle = OpenService(lSCHandle, lServices[lCounter].lpServiceName, SERVICE_CHANGE_CONFIG|SC_MANAGER_ENUMERATE_SERVICE|GENERIC_READ)) != NULL) 
    {
      /*
       * Determine service type
       *
       */
	  
	  ZeroMemory(lServiceType, sizeof(lServiceType));
	  if (lServices[lCounter].ServiceStatusProcess.dwServiceType == SERVICE_FILE_SYSTEM_DRIVER)
        strcpy(lServiceType, "File system driver");
	  else if (lServices[lCounter].ServiceStatusProcess.dwServiceType == SERVICE_KERNEL_DRIVER)
        strcpy(lServiceType, "Kernel driver");
	  else if (lServices[lCounter].ServiceStatusProcess.dwServiceType == SERVICE_WIN32_OWN_PROCESS)
        strcpy(lServiceType, "Own service process");
	  else if (lServices[lCounter].ServiceStatusProcess.dwServiceType == SERVICE_WIN32_SHARE_PROCESS)
        strcpy(lServiceType, "Process sharing service");
	  else if (lServices[lCounter].ServiceStatusProcess.dwServiceType == SERVICE_INTERACTIVE_PROCESS)
        strcpy(lServiceType, "Desktop interacting service");
	  else
        strcpy(lServiceType, "Unknown service");


      if ((lConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, MAX_BUF_SIZE)) != NULL)
	  {
        ZeroMemory(lConfig, MAX_BUF_SIZE);
        QueryServiceConfig(lDetailSCHandle, lConfig, MAX_BUF_SIZE, &lByteNeeded);

        ZeroMemory(lTemp, sizeof(lTemp));
        printf("%-30s %-15s %-30s %-30s\n", lServices[lCounter].lpServiceName, gServiceStatus[lServices[lCounter].ServiceStatusProcess.dwCurrentState], lServiceType, lServices[lCounter].lpDisplayName);
        LocalFree(lConfig);

	  } // if ((lConfig = (LPQUERY...
	  CloseServiceHandle(lDetailSCHandle);
    } // if ((lDetailSCHandle = OpenSer...
  } // for (lCounter = 0; lCounter < lServicesRet...


END:

  if (lServices != NULL)
    LocalFree(lServices);

  return(lRetVal);
}



/*
 * Start a previously stopped service.
 *
 */

DWORD startService(char *pServiceName)
{
  int lRetVal = 0;
  SC_HANDLE lSCManager;
  SC_HANDLE lService;
  int lFuncRetVal = 0;


  /*
   * Get a handle to the SCM database. 
   *
   */

  if ((lSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL) 
  {
    lRetVal = 1;
    goto END;
  }

 
  if ((lService = OpenService(lSCManager, pServiceName, SERVICE_ALL_ACCESS)) == NULL)
  {
    lRetVal = 2;
	goto END;
  }    

  /*
   * Attempt to start the service.
   *
   */

  if (!StartService(lService, 0, NULL))
  {
    lRetVal = 3;
	goto END;
  }


END:

  if (lSCManager)
    CloseServiceHandle(lSCManager);

  if (lService)
    CloseServiceHandle(lService);


  return(lRetVal);
}





/*
 * Stop a running service.
 *
 */

DWORD stopService(char *pServiceName)
{
  DWORD lRetVal = 0;
  SERVICE_STATUS_PROCESS ssp;
  DWORD lwStartTime = GetTickCount();
  DWORD lBytesNeeded;
  DWORD lTimeout = 30000;
  SC_HANDLE lSCManager;
  SC_HANDLE lService;
  int lFuncRetVal = 0;



  /*
   * Get a handle to the SCM database.   
   *
   */

  if ((lSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL) 
  {
    lRetVal = 1;
	goto END;
  }

  /*
   * Get a handle to the service.  
   *
   */

  if ((lService = OpenService(lSCManager, pServiceName, SERVICE_STOP|SERVICE_QUERY_STATUS|SERVICE_ENUMERATE_DEPENDENTS)) == NULL)
  {
    lRetVal = 2; 
	goto END;
  }    


  /*
   * Make sure the service is not already stopped.
   *
   */

  if (!QueryServiceStatusEx(lService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &lBytesNeeded))
  {
    lRetVal = 3; 
    goto END;
  }

  if (ssp.dwCurrentState == SERVICE_STOPPED)
  {
    lRetVal = 4;
	goto END;
  }

  /*
   * If a stop is pending, wait for it.
   *
   */

  while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
  {
    Sleep( ssp.dwWaitHint );
    if (!QueryServiceStatusEx(lService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &lBytesNeeded))
    {
      lRetVal = 5;
      goto END;
    }

    if ( ssp.dwCurrentState == SERVICE_STOPPED )
    {
      lRetVal = 6;
      goto END;
    }


    if (GetTickCount() - lwStartTime > lTimeout)
    {
      lRetVal = 7;
      goto END;
    }
  } // while (ssp.dwCu...



  /*
   * Send a stop code to the service.
   *
   */

  if (!ControlService(lService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS) &ssp))
  {
    lRetVal = 8;
    goto END;
  }



END:

  if (lService) 
    CloseServiceHandle(lService); 

  if (lSCManager)
    CloseServiceHandle(lSCManager);


  return(lRetVal);
}





/*
 * Print tool usage.
 *
 */

void help(char *pToolName)
{
  printf("usage : %s [-h|-l|-c servic-name|-s service-name \n\n", pToolName);
  printf("        %s -h\t\tprint help\n", pToolName);
  printf("        %s -l\t\tlist all services\n", pToolName);
  printf("        %s -c service-name\tcontinue/start a service by its service name\n", pToolName);
  printf("        %s -s service-name\tstop a service by its service name\n", pToolName);
}