//-------------------------------------------------
// This program takes input files and creates a
// resulting histogram and outputs into a file.
// Written by Arda Kıray (21302072)
//-------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/wait.h>
#include <pthread.h>

int *valueArr; // a global array that holds bin values

// A struct argument for threads to execute
struct threadData{
  char *fileName;
  double min;
  double max;
  int binNum;
  int *globalArr;
};

// This function takes the thread argument and process the file.
// It increments the each cell in the globalArr that corresponds to
// a bin number in the histogram by file inputs.
void *threadFunc(void *arg){
  struct threadData *data;
  data = (struct threadData*)arg;
  double minValue = (*data).min;
  double maxValue = (*data).max;
  int bincount = (*data).binNum;

  double w = (maxValue - minValue) / bincount;

  FILE *fp = fopen((*data).fileName, "r");
  
  double temp; // it holds scanned values from the file
  int fileEnd = 0; // character check for end of the file
	  
  // This loop reads each value in the line and calculates
  // which bin it belongs to by rounding it to its floor
  // integer and increments that bin representation in the array
  // by one.
  do{
    if(fscanf(fp, "%lf", &temp) != EOF){

      double result = floor((temp - minValue) / w);
      int temp2 = (int)result;

      // Test if the result is the maxvalue and if it is the maxvalue
      // include into the last bin else use the normal bincount
      if(temp2 == bincount){
	(*data).globalArr[bincount-1]++;
      }
      else{
	(*data).globalArr[temp2]++;
      }
    }	
    // fileEnd becomes one if EOF is encountered
    else{
      fileEnd = 1;
    }
  }while(fileEnd != 1);

  fclose(fp);
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  double minValue;
  double maxValue;
  int bincount;
  int N; // number of files
  char *outFile = (char *)malloc(strlen(argv[argc -1])); // outfile string alloc

  // Below condition detects if enough arguments are supplied
  if(argc < 5){
    printf("Not enough arguments are supplied...\n");
    printf("Program call with proper arguments :\n");
    printf("./phistogram minvalue maxvalue bincount N file1..fileN outfile\n");
    exit(-1);
  }

  // Below codes collect all the command line arguments except file names
  sscanf(argv[1], "%lf", &minValue);
  sscanf(argv[2], "%lf", &maxValue);
  sscanf(argv[3], "%d", &bincount);
  sscanf(argv[4], "%d", &N);
  strcpy(outFile, argv[argc -1]);

  // Below condition detects if N matches with the number of filenames
  // that are supplied
  if((argc-6) != N){
    printf("The file count(N) must match with the number of file names.\n");
    printf("Program call with proper arguments :\n");
    printf("./phistogram minvalue maxvalue bincount N file1..fileN outfile\n");
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

  // size allocation for the global array with 0's
  valueArr = calloc(bincount, sizeof(int));

  pthread_t threads[N]; // thread array
  int threadChecks[N];

  // Below loop creates threads one by one with thread_create and 
  // passes dynamically allocated  threadData structure to each 
  for(int i = 0; i < N; i++){
    struct threadData *pass = malloc(sizeof(struct threadData));
    (*pass).fileName = (char *)malloc(strlen(files[i]));
    strcpy((*pass).fileName, files[i]);
    (*pass).min = minValue;
    (*pass).max = maxValue;
    (*pass).binNum = bincount;
    (*pass).globalArr = valueArr;

    threadChecks[i] = pthread_create(&threads[i], NULL, threadFunc, pass);

    // If thread cannot be created successfully, this condition runs
    if(threadChecks[i]){
      printf("Thread creation failed with code : %d\n", threadChecks[i]);
      exit(-1);
    }
  }

  // Parent thread waits for each thread to finish
  for(int i = 0; i < N; i++){
    pthread_join(threads[i], NULL);
  }

  // This part writes the result into the outfile
  FILE *out = fopen(outFile, "w");
  for(int i = 0; i < bincount; i++){
    fprintf(out, "%d: %d\n", i+1, valueArr[i]); 
  }

  fclose(out); // closing the outfile
}
