#include <windows.h>
#include <stdio.h>
 
 
void main(int argc, char **argv)
{
  FILE *lFH = NULL;
  char lLine[128];
  char lString0[128];
  char lString1[128];
  char lString2[128];
  int lDecimal0 = 0;
  int lDecimal1 = 0;
  char lChar0 = 0;
  char lChar1 = 0;
  int lFRetVal = 0;
 
 
  if (argc > 1 && (lFH = fopen(argv[1],"r")) != NULL)
  {
    ZeroMemory(lLine, sizeof(lLine));
 
    while(fgets(lLine, sizeof(lLine), lFH) != NULL)
    {
      /*
       * Remove trailing NL/CR characters
       */
      while (lLine[strlen(lLine)-1] == '\r' || lLine[strlen(lLine)-1] == '\n')
        lLine[strlen(lLine)-1] = '\0';
 
      /*
       * 
       */
      if ((lFRetVal = sscanf(lLine, "%[^:]:%c:%d:%d:%s", lString0, &lChar0, &lDecimal0, &lDecimal1, lString2)) == 5)
        printf("%s - %c - %d - %d - %s\n", lString0, lChar0, lDecimal0, lDecimal1, lString2);
      else if ((lFRetVal = sscanf(lLine, "%[^:]:%s", lString0, lString1)) == 2)
        printf("%s - %s\n", lString0, lString1);
    }
 
    fclose(lFH);
  }
}


/*
* Config file content
* proxy.intranet:192.168.1.100
* admin:x:33:312:/bin/bash  
*/