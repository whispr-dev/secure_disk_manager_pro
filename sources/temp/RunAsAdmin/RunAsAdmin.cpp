#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>


/*
 * Function forward declarations.
 *
 */
int UserIsAdmin();
BOOL SetCurrentPrivilege (LPCTSTR pszPrivilege, BOOL bEnablePrivilege);
BOOL GetProcnameByID(int pPID, char *pOutput, int pOutputLen);
int GetParentPIDByPID(int pPID);



/* 
 * Program entry point.
 *
 */
int main(int argc, char *argv[])
{
  BOOL lIsAdmin = FALSE;
  char lPrivileges[1024] = {0};
  char lParentExeFileName[1024] = {0};
  char lExeFileName[1024] = {0};
  DWORD lCurrPID = GetCurrentProcessId();
  int lParentPID = GetParentPIDByPID(lCurrPID);
  SHELLEXECUTEINFO lShExecInfo = {0};

  /*
   * Check privileges
   */
  SetCurrentPrivilege (SE_DEBUG_NAME, TRUE);
  lIsAdmin = UserIsAdmin();
  strcpy(lPrivileges, lIsAdmin?"Admin":"!Admin");


  /*
   * Get proces and parent process exe file name.
   */
  GetProcnameByID(lParentPID, lParentExeFileName, sizeof(lParentExeFileName));
  GetProcnameByID(lCurrPID, lExeFileName, sizeof(lExeFileName));
  

  /*
   * We are not admin and the parent is not an instance of ourself 
   * -> create another instance with 
   */
  if (!lIsAdmin && strncmp(lExeFileName, lParentExeFileName, sizeof(lExeFileName)))
  {
    printf("main(1.1) : Privs: %s - Proc: %s(%i) - Parent: %s(%i)\n", lPrivileges, lExeFileName, lCurrPID, lParentExeFileName, lParentPID);

    lShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    lShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    lShExecInfo.hwnd = NULL;
    lShExecInfo.lpVerb = "runas";
    lShExecInfo.lpFile = argv[0];		
    lShExecInfo.lpParameters = NULL;	
    lShExecInfo.lpDirectory = NULL;
    lShExecInfo.nShow = SW_SHOWNORMAL;
    lShExecInfo.hInstApp = NULL;	
    ShellExecuteEx(&lShExecInfo);
    WaitForSingleObject(lShExecInfo.hProcess, INFINITE);

  /*
   * We are admin. 
   */
  }
  else if (lIsAdmin)
  {
    printf("main(1.2) : Privs: %s - Proc: %s(%i) - Parent: %s(%i)\n", lPrivileges, lExeFileName, lCurrPID, lParentExeFileName, lParentPID);
    getchar();


  /*
   * We are still not admin. 
   */
  }
  else
  {
     printf("main(1.3) : Privs: %s - Proc: %s(%i) - Parent: %s(%i)\n", lPrivileges, lExeFileName, lCurrPID, lParentExeFileName, lParentPID);
  }
}


/*
 *
 *
 */
int UserIsAdmin()
{
  BOOL lRetVal = FALSE;
  SID_IDENTIFIER_AUTHORITY lNtAuthority = SECURITY_NT_AUTHORITY;
  PSID lAdmGroup = NULL;


  if(AllocateAndInitializeSid(&lNtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &lAdmGroup)) 
  {
    if (! CheckTokenMembership(NULL, lAdmGroup, &lRetVal))
      lRetVal = FALSE;

    FreeSid(lAdmGroup); 
  }

  return(lRetVal);
}





/*
 *
 *
 */
BOOL GetProcnameByID(int pPID, char *pOutput, int pOutputLen)
{
  BOOL lRetVal = FALSE;
  HANDLE lProcessSnap;
  PROCESSENTRY32 lProcEntry;
  

  if (pPID > 0 && pOutput != NULL && pOutputLen > 0)
  {
    if((lProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE)
    {
      lProcEntry.dwSize = sizeof(PROCESSENTRY32);
      if (Process32First(lProcessSnap, &lProcEntry))
      {
        do
        {
          if (lProcEntry.th32ProcessID == pPID)
          {
            strncpy(pOutput, lProcEntry.szExeFile, pOutputLen-1);
            lRetVal = TRUE;
            break;
          } // if (lProcEntry...   
        } while(Process32Next(lProcessSnap, &lProcEntry));
      } // if (Process32First(...
      CloseHandle(lProcessSnap);
    } // if((lProcessSnap = ...
  } // if (pPID > 0 &&...

  return(lRetVal);
}



/*
 *
 *
 */
int GetParentPIDByPID(int pPID)
{
  int lRetVal = -1;
  HANDLE hProcessSnap;
  PROCESSENTRY32 lProcEntry;


  if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE)
  {
    lProcEntry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hProcessSnap, &lProcEntry))
    {
      do
      {
        if (lProcEntry.th32ProcessID == pPID)
        {
          lRetVal = lProcEntry.th32ParentProcessID;
          break;
        } // if (lProcEntry...  
      } while(Process32Next(hProcessSnap, &lProcEntry));
    } // if (Process32First(...
    CloseHandle(hProcessSnap);
  } // if ((hProcessSnap...

  return(lRetVal);
}



/*
 *
 *
 */
BOOL SetCurrentPrivilege (LPCTSTR pPrivilege, BOOL bEnablePrivilege)
{
  BOOL lRetVal = FALSE;
  HANDLE hToken;
  TOKEN_PRIVILEGES lPrivToken;
  LUID lLUID;
  TOKEN_PRIVILEGES lPrevPrivToken;
  DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);
  

  if (LookupPrivilegeValue(NULL, pPrivilege, &lLUID))
  {

    /*
     * first pass.  get current privilege setting
     */
    if (OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
      lPrivToken.PrivilegeCount           = 1;
      lPrivToken.Privileges[0].Luid       = lLUID;
      lPrivToken.Privileges[0].Attributes = 0;
  

      /*
       * second pass.  set privilege based on previous setting
       */
      if (AdjustTokenPrivileges(hToken, FALSE, &lPrivToken, sizeof(TOKEN_PRIVILEGES), &lPrevPrivToken, &cbPrevious)) 
      {	
        lPrevPrivToken.PrivilegeCount     = 1;
        lPrevPrivToken.Privileges[0].Luid = lLUID;

        if(bEnablePrivilege)
          lPrevPrivToken.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
        else
          lPrevPrivToken.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & lPrevPrivToken.Privileges[0].Attributes);

        if (AdjustTokenPrivileges(hToken, FALSE, &lPrevPrivToken, cbPrevious, NULL, NULL))
          lRetVal=TRUE;
      }

      CloseHandle(hToken);
    }
  }

  return(lRetVal);
}