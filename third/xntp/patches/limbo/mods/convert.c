
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>


#include "ntp_io.h"
#include "ntp_unixtime.h"
#include "l_stdlib.h"



main(){

  int  yday;
  int  hour;
  int  minute;
  int  second;
  int  tzoff;
  int  i;
  FILE*     fdToRead;
  u_long    rec_ui;
  u_long    yearstart;
  u_int32   ts_ui; 
  
  printf("CONVERTER STARTED \n\n");

  fdToRead = fopen("ntp.out","r");
  fscanf(fdToRead, "%d %d %d %d %d",&yday, &hour, &minute, &second, &tzoff);  
  fscanf(fdToRead, "%d %d %d", &rec_ui, &yearstart, &ts_ui);
  fclose(fdToRead);

  printf("yday      = %d\n",yday); 
  printf("hour      = %d\n",hour); 
  printf("minute    = %d\n",minute); 
  printf("second    = %d\n",second); 
  printf("time zone = %d\n",tzoff);
  printf("rec_ui    = %u\n",rec_ui);
  printf("yearstart = %u\n",yearstart);
  printf("ts_ui     = %u\n",ts_ui);

  printf("CALLING CLOCKTIME \n\n");
  i = clocktime(yday, hour, minute, second, tzoff, rec_ui, &yearstart, &ts_ui);

  printf("yday      = %d\n\n",yday); 
  printf("hour      = %d\n",hour); 
  printf("minute    = %d\n",minute); 
  printf("second    = %d\n",second); 
  printf("time zone = %d\n",tzoff);
  printf("rec_ui    = %u\n",rec_ui);
  printf("yearstart = %u\n",yearstart);
  printf("ts_ui     = %u\n\n",ts_ui);

  printf("CLOCKTIME RETURNED %d\n",i);

  if (!i)
     printf ("Error condition \n");

}
