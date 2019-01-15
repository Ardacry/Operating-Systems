//-----------------------------------------------
// This program takes N number of input files and
// creates a resulting histogram from the inputs
// by creating and assembling N intermediate files.
// The result is put into an output file.
// Written by Arda Kıray (21302072)
//-----------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
  double minvalue;
  double maxvalue;
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
  sscanf(argv[1], "%lf", &minvalue);
  sscanf(argv[2], "%lf", &maxvalue);
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
  
  double w = (maxvalue - minvalue) / bincount; //bin-width

  int idArray[N]; // an array that collects the ids of child processes

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

	// Intermediate file creation with int suffix
	strcat(fileName, "int");
	FILE *new = fopen(fileName, "w");
	
	for(int i = 0; i < bincount; i++){
	  fprintf(new,"%d: %d\n", i+1, valueArr[i]); 
	}
	
	fclose(new); // closing the intermediate file
	exit(0); 
      }

      fileCount++;
  }

  // In this part, parent process waits for child processes to finish and
  // once they finished, combine the result into outfile
  for(int i = 0; i < N; i++){
    wait(NULL);
  }
  
  FILE *curr; // pointer to currently processed fileN
  int resultArr[bincount]; // the cumulative results are held in this array

  // Initializing the resultArr to zeros before its use
  for(int j = 0; j < bincount; j++){
    resultArr[j] = 0;
  } 

  // This loop processes all intermediate files that are created by child
  // processes takes the binvalues one by one and increment the resultArr's
  // corresponding cell with the values taken from the files.
  int fileCnt = 0;
  for(int i = 1; i <= N; i++){
    char *fName = (char *)malloc(strlen(files[fileCnt]));
    strcpy(fName, files[fileCnt]);
    strcat(fName, "int");

    curr = fopen(fName, "r");
    
    int binResult; // the value at the currently processed bin
    int iteration = 0; // iteration count to determine bin number
    int bin = 0; // bin number
    int eof = 0; // End of file indicator to terminate the loop

    // This loop iterates among the file until EOF is encountered and
    // increment the resultArr's values.
    do{
       eof = fscanf(curr,"%d: %d/n", &bin, &binResult);
       if(eof != EOF){
	 resultArr[iteration] = resultArr[iteration] + binResult;
	 iteration++;
       }
    }while(eof != EOF);

    fileCnt++;
    fclose(curr);
  }

  // This part writes the result into the outfile
  FILE *out = fopen(outFile, "w");
  for(int i = 0; i < bincount; i++){
    fprintf(out, "%d: %d\n", i+1, resultArr[i]); 
  }

  fclose(out); // closing the outfile
}


	  
	  

	

	

	
        
	
      
  
