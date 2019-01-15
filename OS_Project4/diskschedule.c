//-----------------------------------------------
// This program simulates the disk scheduling
// algorithms : FCFS, SSTF, SCAN, C-SCAN, LOOK
// and C-LOOK by serving 1000 requests that either
// given as an input file or created randomly.
// Author : Arda Kıray (21302072)
//-----------------------------------------------
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// Random disk request generator function
void randGenerator(int* arr){
  srand(time(0));
  for(int i = 0; i < 1000; i++){
    int num = rand() % 4999 + 0;
    arr[i] = num;
  }
}

int main(int argc, char *argv[])
{
  int requestArr[1000] = {0};
  int headpos;
  char *inFile;
  

  // Below condition detects if enough arguments are supplied
  if(argc < 2){
    printf("Not enough arguments are supplied...\n");
    printf("Program call with proper arguments :\n");
    printf("./diskschedule <headpos> or ./diskschedule <headpos> <inputfile>\n");
    exit(-1);
  }

  if(argc > 3){
    printf("Erroneous program call...\n");
    printf("Program call with proper arguments :\n");
    printf("./diskschedule <headpos> or ./diskschedule <headpos> <inputfile>\n");
    exit(-1);
  }

  // If an input file is supplied, below loop reads the file
  // and puts the disk number into the requestArr[].
  if(argc == 3){
     inFile = (char *)malloc(strlen(argv[argc -1])); // inFile string alloc
     strcpy(inFile, argv[argc -1]);

     FILE *fp = fopen(inFile, "r");
     int requestNum;
     int diskNum;
     int fileEnd = 0;
     do{
       if(fscanf(fp, "%d %d", &requestNum, &diskNum) != EOF){
	 requestArr[requestNum-1] = diskNum;
       }
       // fileEnd becomes one if EOF is encountered
       else{
	 fileEnd = 1;
       }
     }while(fileEnd != 1);
  }
  else{
    randGenerator(requestArr);
  }
   
  sscanf(argv[1], "%d", &headpos);

  if(headpos > 4999){
    printf("Head-position must be in [0,4999]...\n");
    exit(-1);
  }

  /* 
               FCFS ALGORITHM :
  */
  
  int FCFS_movement = 0; // total resulting head movement
  if(requestArr[0] > headpos){
    int temp = requestArr[0] - headpos;
    FCFS_movement += temp;
  }
  else{
    int temp = headpos - requestArr[0];
    FCFS_movement += temp;
  }
  for(int i = 1; i < 1000; i++){
    if(requestArr[i-1] > requestArr[i]){
      int temp = requestArr[i-1] - requestArr[i];
      FCFS_movement += temp;
    }
    else{
      int temp = requestArr[i] - requestArr[i-1];
      FCFS_movement += temp;
    }
  }

  /* 
                SSTF ALGORITHM :
  */
  // Array copying to protect the original request array
  int sstfArr[1000];
  for(int i = 0; i < 1000; i++){
    sstfArr[i] = requestArr[i];
  }
  
  int SSTF_movement = 0;
  int minIndex = 0; // current minimum movement request index
  int minValue = 0; // current minimum movement 
  int current = headpos; // current request to be processed
  int counter = 0;

  // First minValue initialization
  if(current > sstfArr[0])
    minValue = current - requestArr[0];
  else
    minValue = sstfArr[0] - current;

  // Below loop compares the every requests' movement size
  // with the current head position and picks the smallest
  // one. If two of the smallest exists, it picks the right
  // side one.
  while(counter < 1000){
    for(int i = 0; i < 1000; i++){
      int tempValue = 5000;
      if(sstfArr[i] != -1){
	if(current > sstfArr[i])
	  tempValue = current - sstfArr[i];
	else
	  tempValue = sstfArr[i] - current;

	// If equal movement, go the right(bigger number).
	if(tempValue == minValue){
	  if(minIndex < i)
	    minIndex = i;
	}

	if(tempValue < minValue){
	  minValue = tempValue;
	  minIndex = i;
	}
      }
    }

    SSTF_movement += minValue;
    current = sstfArr[minIndex]; // current head is on the granted request
    sstfArr[minIndex] = -1; // indicates that the request is granted.
    int found = 0;
    int counter2 = -1;

    // MinValue pick for the first non-granted request
    while(found != 1){
      counter2++;
      if(sstfArr[counter2] != -1){
	if(current > sstfArr[counter2])
	  minValue = current - sstfArr[counter2];
	else
	  minValue = sstfArr[counter2] - current;
	found = 1;
      }
    }
    minIndex = counter2;
    counter++;
  }

  /*
               SCAN ALGORITHM :
  */
  // Protecting the requestArr by copying it.
  int scanArr[1000];
  for(int i = 0; i < 1000; i++)
    scanArr[i] = requestArr[i];

  int scan_curr = headpos; // current head position
  int scan_right_finished = 0;
  int scan_finished = 0;
  int SCAN_movement = 0;

  // Right scan loop that increments the current head position
  // by 1 in each iteration. By the time head position iterates
  // the requests on the way are served and put -1 values in their
  // corresponding values. If the requests finish before right scan
  // finishes, loop terminates.
  while(scan_right_finished != 1 && scan_finished != 1){
    scan_finished = 1;

    //Check each request if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(scan_curr == scanArr[i])
	scanArr[i] = -1;
    }

    // Check if the all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(scanArr[j] != -1)
	scan_finished = 0;	
    }
    
    if(scan_curr == 4999)
      scan_right_finished = 1;

    if(scan_right_finished != 1 && scan_finished != 1){
      scan_curr++;
      SCAN_movement++;
    }
  }

  // Left scan loop that decrements the current head position by 1
  // in each iteration. Scan either finishes if no requests left or
  // left-scan finishes at disk index 0.
  while(scan_finished != 1){
    scan_finished = 1;

    // Check each requests if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(scan_curr == scanArr[i])
	scanArr[i] = -1;
    }

    // Check if all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(scanArr[j] != -1)
	scan_finished = 0;	
    }

    if(scan_curr == 0)
      scan_finished = 1;

    if(scan_finished != 1){
      scan_curr--;
      SCAN_movement++;
    }
    
  }

  /*
                  C-SCAN ALGORITHM :
  */
  // Protecting the requestArr by copying it.
  int cscanArr[1000];
  for(int i = 0; i < 1000; i++)
    cscanArr[i] = requestArr[i];

  int cscan_curr = headpos; // current head position
  int cscan_right_finished = 0;
  int cscan_finished = 0;
  int CSCAN_movement = 0;

  // Right scan loop that increments the current head position
  // by 1 in each iteration. By the time head position iterates
  // the requests on the way are served and put -1 values in their
  // corresponding values. If the requests finish before right scan
  // finishes, loop terminates.
  while(cscan_right_finished != 1 && cscan_finished != 1){
    cscan_finished = 1;

    //Check each request if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(cscan_curr == cscanArr[i])
	cscanArr[i] = -1;
    }

    // Check if the all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(cscanArr[j] != -1)
	cscan_finished = 0;	
    }
    
    if(cscan_curr == 4999)
      cscan_right_finished = 1;

    if(cscan_finished != 1 && cscan_right_finished != 1){
      cscan_curr++;
      CSCAN_movement++;
    }
  }

  // C-SCAN loop back to the starting end point
  if(cscan_finished != 1){
    cscan_curr = 0;
    CSCAN_movement += 4999;
    cscan_right_finished = 0;
  }

  // Another right scan loop to finish the remainings
  while(cscan_right_finished != 1 && cscan_finished != 1){
    cscan_finished = 1;

    //Check each request if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(cscan_curr == cscanArr[i])
	cscanArr[i] = -1;
    }

    // Check if the all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(cscanArr[j] != -1)
	cscan_finished = 0;	
    }
    
    if(cscan_curr == 4999)
      cscan_right_finished = 1;

    if(cscan_finished != 1 && cscan_right_finished != 1){
      cscan_curr++;
      CSCAN_movement++;
    }
  }

  /*
                 LOOK ALGORITHM :
  */
  // Protecting the requestArr by copying it.
  int lookArr[1000];
  for(int i = 0; i < 1000; i++)
    lookArr[i] = requestArr[i];

  int look_curr = headpos; // current head position
  int look_right_finished = 0;
  int look_finished = 0;
  int LOOK_movement = 0;

  // Right scan loop that increments the current head position
  // by 1 in each iteration. By the time head position iterates
  // the requests on the way are served and put -1 values in their
  // corresponding values. If the requests finish before right scan
  // finishes, loop terminates.
  while(look_right_finished != 1 && look_finished != 1){
    look_finished = 1;
    look_right_finished = 1;

    //Check each request if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(look_curr == lookArr[i])
	lookArr[i] = -1;
    }

    // Check if the all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(lookArr[j] != -1)
	look_finished = 0;	
    }

    // Loop to check if all right-side requests are met.
    for(int k = 0; k < 1000; k++){
      if(look_curr < lookArr[k])
	look_right_finished = 0;
    }

    if(look_right_finished != 1 && look_finished != 1){
      look_curr++;
      LOOK_movement++;
    }
  }

  // Left scan loop that decrements the current head position by 1
  // in each iteration. Scan either finishes if no requests left or
  // left-scan finishes at the last request.
  while(look_finished != 1){
    look_finished = 1;

    // Check each requests if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(look_curr == lookArr[i])
	lookArr[i] = -1;
    }

    // Check if all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(lookArr[j] != -1)
	look_finished = 0;	
    }

    // Loop to check if all left-side requests are met.
    for(int k = 0; k < 1000; k++){
      if(look_curr > lookArr[k] && lookArr[k] != -1)
	look_finished = 0;
    }

    if(look_finished != 1){
      look_curr--;
      LOOK_movement++;
    }
    
  }

  /*
                  C-LOOK ALGORITHM :
  */
  // Protecting the requestArr by copying it.
  int clookArr[1000];
  for(int i = 0; i < 1000; i++)
    clookArr[i] = requestArr[i];

  int clook_curr = headpos; // current head position
  int clook_right_finished = 0;
  int clook_finished = 0;
  int CLOOK_movement = 0;

  // Right scan loop that increments the current head position
  // by 1 in each iteration. By the time head position iterates
  // the requests on the way are served and put -1 values in their
  // corresponding values. If the requests finish before right scan
  // finishes, loop terminates.
  while(clook_right_finished != 1 && clook_finished != 1){
    clook_finished = 1;
    clook_right_finished = 1;

    //Check each request if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(clook_curr == clookArr[i])
	clookArr[i] = -1;
    }

    // Check if the all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(clookArr[j] != -1)
	clook_finished = 0;	
    }

    // Loop to check if all right-side requests are met.
    for(int k = 0; k < 1000; k++){
      if(clook_curr < clookArr[k])
	clook_right_finished = 0;
    }

    if(clook_right_finished != 1 && clook_finished != 1){
      clook_curr++;
      CLOOK_movement++;
    }
  }

  // C-LOOK loop back to the starting end.
  if(clook_finished != 1){
    int minValue;
    for(int k = 0; k < 1000; k++){
      if(clookArr[k] != -1)
	minValue = clookArr[k];
    }
    for(int i = 0; i < 1000; i++){
      if(clookArr[i] < minValue && clookArr[i] != -1)
	minValue = clookArr[i];
    }
  
    CLOOK_movement += (clook_curr - minValue);
    clook_curr = minValue;
    clook_right_finished = 0;
  }

  // Another right scan loop to finish the remainings
  while(clook_right_finished != 1 && clook_finished != 1){
    clook_finished = 1;
    clook_right_finished = 1;

    //Check each request if it matches with current.
    for(int i = 0; i < 1000; i++){
      if(clook_curr == clookArr[i])
	clookArr[i] = -1;
    }

    // Check if the all requests are granted.
    for(int j = 0; j < 1000; j++){
      if(clookArr[j] != -1)
	clook_finished = 0;	
    }

    // Loop to check if all right-side requests are met.
    for(int k = 0; k < 1000; k++){
      if(clook_curr < clookArr[k])
	clook_right_finished = 0;
    }

    if(clook_right_finished != 1 && clook_finished != 1){
      clook_curr++;
      CLOOK_movement++;
    }
  }

  // Below code writes the results into the output file.
  FILE *out = fopen("output", "w");
  fprintf(out, "FCFS: %d\n", FCFS_movement);
  fprintf(out, "SSTF: %d\n", SSTF_movement);
  fprintf(out, "SCAN: %d\n", SCAN_movement);
  fprintf(out, "C-SCAN: %d\n", CSCAN_movement);
  fprintf(out, "LOOK: %d\n", LOOK_movement);
  fprintf(out, "C-LOOK: %d\n", CLOOK_movement);
  fclose(out);
}
