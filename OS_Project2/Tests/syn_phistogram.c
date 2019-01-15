//-----------------------------------------------
// This program takes N number of input files and
// creates a resulting histogram from the inputs
// files by multi-processing.
// The result is put into an output file.
// Author : Arda Kıray (21302072)
//-----------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  double minvalue;
  double maxvalue;
  int bincount;
  int N; // number of files
  char *outFile = (char *)malloc(strlen(argv[argc -1])); // outfile string alloc
  int shm_fd; // shared memory file descriptor
  int *shm; // shared memory pointer
  const char *memName = "Shared"; // name of the shared memory
  sem_t bin_semaphore; // semaphore declaration
  sem_t bin_semaphore2;


  // Below condition detects if enough arguments are supplied
  if(argc < 5){
    printf("Not enough arguments are supplied...\n");
    printf("Program call with proper arguments :\n");
    printf("./syn_phistogram minvalue maxvalue bincount N file1..fileN outfile\n");
    exit(-1);
  }

  // Below codes collect all the command line arguments except file names
  sscanf(argv[1], "%lf", &minvalue);
  sscanf(argv[2], "%lf", &maxvalue);
  sscanf(argv[3], "%d", &bincount);
  sscanf(argv[4], "%d", &N);
  strcpy(outFile, argv[argc -1]);

  int sharedArr[bincount]; // shared memory array creation

  // Below condition detects if N matches with the number of filenames
  // that are supplied
  if((argc-6) != N){
    printf("The file count(N) must match with the number of file names.\n");
    printf("Program call with proper arguments :\n");
    printf("./syn_phistogram minvalue maxvalue bincount N file1..fileN outfile\n");
    exit(-1);
  }
    
  char *files[N]; // string array that holds the name of given files

  // This loop reads the names of files and allocates the needed amount
  // of memory for writing each filename into files array
  int argIterator = 0; // iterator for number of files
  for(int i = 5; i < (argc - 1); i++){
    files[argIterator] = (char *)malloc(strlen(argv[i]));
    strcpy(files[argIterator], argv[i]);
    argIterator++;
  }
  
  double w = (maxvalue - minvalue) / bincount; //bin-width

  int idArray[N]; // an array that collects the ids of child processes
  int memSize = (bincount * sizeof(int)); // shared memory size allocation

  // Shared memory portion opening 
  shm_fd = shm_open(memName, O_CREAT | O_RDWR, 0666);
  if(shm_fd < 0){
    printf("Shared memory cannot be created...\n");
    exit(1);
  }
  ftruncate(shm_fd, memSize); // memory size initilization
  shm = (int *)mmap(NULL, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if(shm < 0){
    printf("Shared memory cannot be mapped...\n");
    exit(1);
  }
  close(shm_fd);

  int ret = sem_init(&bin_semaphore, 1, 1); // opening semaphore
  int ret2 = sem_init(&bin_semaphore, 0, 1);
  if(ret  == -1){
    printf("Semaphore is failed to be created...\n");
    exit(-1);
  }

  // This loop iterates among all N files and forks a child for each file
  // to be processed by a child process.
  int fileCount = 0;// counter for the currently processed file number
  for(int i = 0; i < N; i++){
      idArray[i] = fork();

      // Check if child process is created succesfully
      if(idArray[i] < 0){
	printf("Fork Failed\n");
	exit(-1);
      }
      // Child process code
      else if(idArray[i] == 0){
	
	// Get the current file's name from file name array
	char *fileName = (char *)malloc(strlen(files[fileCount]));
	strcpy(fileName, files[fileCount]);
	FILE *fp = fopen(fileName, "r");
	  
	// initialize an array for bin occurences with zeros
	int valueArr[bincount];
	for(int i = 0; i < bincount; i++){
	  valueArr[i] = 0;
	}
	
	double temp; // it holds scanned values from the file
	int fileEnd = 0; // character check for end of the file
	  
	// This loop reads each value in the line and calculates
	// which bin it belongs to by rounding it to its floor
	// integer and increments that bin representation in the array
	// by one.
	do{
	  if(fscanf(fp, "%lf", &temp) != EOF){

	    double result = floor((temp - minvalue) / w);
	    int temp2 = (int)result;

	    // Test if the result is the maxvalue and if it is the maxvalue
	    // include into the last bin else use the normal bincount
	    if(temp2 == bincount){
	      valueArr[bincount-1]++;
	    }
	    else
	      valueArr[temp2]++;
	  }
	  // fileEnd becomes one if EOF is encountered
	  else{
	    fileEnd = 1;
	  }
	}while(fileEnd != 1);

	fclose(fp); // closing the processed file

	// Below loop iterates through the shared memory and gather respected
	// values one by one. Example shm_ptr[0] has the 1st bin number element
	// count. Synchronization of processes is ensured by a binary semaphore.
	int *shm_ptr = shm;
	sem_wait(&bin_semaphore);
	for(int i = 0; i < bincount; i++){ //---------------------
	  shm_ptr[i] = valueArr[i];        // Critical Section
	}                                  //--------------------- 
	sem_post(&bin_semaphore);
	sem_wait(&bin_semaphore2);
    
	exit(0); 
      }

      fileCount++;
  }

  int resultArr[bincount]; // the cumulative results are held in this array

  // Initializing the resultArr to zeros before its use
  for(int j = 0; j < bincount; j++){
    resultArr[j] = 0;
  } 

  // In this part, parent process waits for a child process to finish. After
  // one child process finishes, parent process reads the results into its
  // array and increment the number accordingly. Once it finishes, parent
  // process waits for another child process to finish.
  for(int i = 0; i < N; i++){
    wait(NULL);
    
    int *mem_ptr = shm;
    sem_wait(&bin_semaphore);
    for(int j = 0; j < bincount; j++){    //------------------------
      int val = 0;                        //  Critical Section
      val = mem_ptr[j];                   //------------------------  
      resultArr[j] = resultArr[j] + val;
    }
    sem_post(&bin_semaphore);
    sem_post(&bin_semaphore2);
  }

  shm_unlink(memName); // unlinking the shared memory area
  sem_destroy(&bin_semaphore); // destroy the semaphore

  // This part writes the result into the outfile
  FILE *out = fopen(outFile, "w");
  for(int i = 0; i < bincount; i++){
    fprintf(out, "%d: %d\n", i+1, resultArr[i]); 
  }

  fclose(out); // closing the outfile
}
