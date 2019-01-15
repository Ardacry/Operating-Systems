//-----------------------------------------------
// This program creates N number of files filled 
// with randomly generated integers in the bounds
// of [min,max] that are provided. The size of the
// files and their names are also provided by the
// user as a command line argument.
// Author : Arda Kıray (21302072)
//-----------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int min;
  int max;
  int size;
  int fileNum;

  // Below condition detects if enough arguments are supplied
  if(argc < 5){
    printf("Not enough arguments are supplied...\n");
    printf("Program call with proper arguments :\n");
    printf("./FileCreator minvalue maxvalue size fileCount file1..fileN\n");
    exit(-1);
  }

  // Below codes collect all the command line arguments except file names
  sscanf(argv[1], "%d", &min);
  sscanf(argv[2], "%d", &max);
  sscanf(argv[3], "%d", &size);
  sscanf(argv[4], "%d", &fileNum);

  // Below condition detects if N matches with the number of filenames
  // that are supplied
  if((argc-5) != fileNum){
    printf("The file count must match with the number of file names.\n");
    printf("Program call with proper arguments :\n");
    printf("./FileCreator minvalue maxvalue size fileCount file1..fileN\n");
    exit(-1);
  }

  char *files[fileNum];

  // This loop reads the names of files and allocates the needed amount
  // of memory for writing each filename into files array
  int argIterator = 0; // iterator for number of files
  for(int i = 5; i < argc; i++){
    files[argIterator] = (char *)malloc(strlen(argv[i]));
    strcpy(files[argIterator], argv[i]);
    argIterator++;
  }
  
  FILE *fp;

  // This loop creates desired amount of files with given sizes and
  // fills them with randomly generated integers in the bounds of
  // [min,max] provided
  for(int i = 0; i < fileNum; i++){
    fp  = fopen(files[i], "w");

    srand(time(0));

    for(int i = 0; i < size; i++){
      int num = rand() % max + min;
      fprintf(fp, "%d %d\n", i+1, num);
    }

    fclose(fp);
  }
}
      
      
  
