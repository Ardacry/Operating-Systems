//-------------------------------------------------
// This program takes input files and creates a
// resulting histogram and outputs into a file.
// Author : Arda Kıray (21302072)
//-------------------------------------------------
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
#include <pthread.h>

pthread_mutex_t globMutex1; // binary mutex1
pthread_mutex_t exitMutex; // binary mutex2 for the number of finished threads
pthread_cond_t batchCond; // conditional variable to signal when batch I/O finished
int exitNum = 0; // number of finished threads
int signalCount = 0; // count of the provided batches by threads

// Node structure for the global linked-list
struct Node{
  double data;
  struct Node *next;
};

// Item adder function for global linked list. It adds at the head of the list
void addItem(struct Node **head, double item){
  struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));

  newNode->data = item;
  newNode->next = *head;
  *head = newNode;
}

// This function removes the first item in the linked list
double pop(struct Node **head){
  if(*head == NULL){
    return -1;
  }

  struct Node *nextNode = (*head)->next;
  double value = (*head)->data;
  free(*head);
  *head = nextNode;

  return value;
}

struct Node *list; // global linked list declaration

// A struct argument for threads to execute
struct threadData{
  char *fileName;
  int batchNum;
};

// This function takes the thread argument and process the file.
// It takes the input from file with B batches that supported
// in its argument. It then atomically, uploads those B batches
// into the global linked list. 
void *threadFunc(void *arg){
  struct threadData *data;
  data = (struct threadData*)arg;
  int batch = (*data).batchNum;

  FILE *fp = fopen((*data).fileName, "r");
  
  double temp; // it holds scanned values from the file
  int fileEnd; // character check for end of the file
  double batchArr[batch]; // batch array that holds the batch numbers 
	  
  // This loop reads each value in the line and calculates
  // which bin it belongs to by rounding it to its floor
  // integer and increments that bin representation in the array
  // by batches of B.
  do{
      int counter = 0;
      while((fileEnd != EOF) && (counter < batch)){
	fileEnd = fscanf(fp, "%lf", &temp);
	if(fileEnd != EOF){
	  batchArr[counter] = temp;
	  counter++;
	}
      }

      // Mutex lock before the addition of the batch into the list
      // Critical section begins
      pthread_mutex_lock(&globMutex1);
      for(int i = 0; i < counter; i++){
	addItem(&list, batchArr[i]);
      }
      signalCount++; // increment the number of times a batch finish signaled
      pthread_cond_signal(&batchCond); // signal the finish of the batch B
      pthread_mutex_unlock(&globMutex1);
      //Critical section ends
	
      if(fileEnd == EOF)
	fileEnd = -5;
  }while(fileEnd != -5);

  fclose(fp);

  // Mutex lock to protect the number of finished threads
  // Critical section begins
  pthread_mutex_lock(&exitMutex);
  exitNum++; // finished thread increment the number
  signalCount++; //In order to prevent the deadlock, each finishing thread signal its finish.
  pthread_cond_signal(&batchCond);
  pthread_mutex_unlock(&exitMutex);
  // Critical section ends
  
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  double minValue;
  double maxValue;
  int bincount;
  int N; // number of files
  char *outFile = (char *)malloc(strlen(argv[argc -2])); // outfile string alloc
  int batch;
  
  // Below condition detects if enough arguments are supplied
  if(argc < 5){
    printf("Not enough arguments are supplied...\n");
    printf("Program call with proper arguments :\n");
    printf("./syn_thistogram minvalue maxvalue bincount N file1..fileN outfile B\n");
    exit(-1);
  }

  // Below codes collect all the command line arguments except file names
  sscanf(argv[1], "%lf", &minValue);
  sscanf(argv[2], "%lf", &maxValue);
  sscanf(argv[3], "%d", &bincount);
  sscanf(argv[4], "%d", &N);
  strcpy(outFile, argv[argc -2]);

  // Below condition detects if N matches with the number of filenames
  // that are supplied
  if((argc-7) != N){
    printf("The file count(N) must match with the number of file names.\n");
    printf("Program call with proper arguments :\n");
    printf("./syn_thistogram minvalue maxvalue bincount N file1..fileN outfile B\n");
    exit(-1);
  }

  sscanf(argv[argc-1], "%d", &batch); // batch number argument
  if(batch < 1 || batch > 100){
    printf("Batch number must be between [1,100]...\n");
    exit(-1);
  }

  char *files[N]; // string array that holds the name of given files

  // This loop reads the names of files and allocates the needed amount
  // of memory for writing each filename into files array
  int argIterator = 0; // iterator for number of files
  for(int i = 5; i < (argc - 2); i++){
    files[argIterator] = (char *)malloc(strlen(argv[i]));
    strcpy(files[argIterator], argv[i]);
    argIterator++;
  }

  // Initilization of mutexes and condition variable
  pthread_mutex_init(&globMutex1, NULL);
  pthread_mutex_init(&exitMutex, NULL);
  pthread_cond_init(&batchCond, NULL);
  pthread_t threads[N]; // thread array
  int threadChecks[N];

  // Below loop creates threads one by one with thread_create and 
  // passes dynamically allocated  threadData structure to each 
  for(int i = 0; i < N; i++){
    struct threadData *pass = malloc(sizeof(struct threadData));
    (*pass).fileName = (char *)malloc(strlen(files[i]));
    strcpy((*pass).fileName, files[i]);
    (*pass).batchNum = batch;

    threadChecks[i] = pthread_create(&threads[i], NULL, threadFunc, pass);

    // If thread cannot be created successfully, this condition runs
    if(threadChecks[i]){
      printf("Thread creation failed with code : %d\n", threadChecks[i]);
      exit(-1);
    }
  }

  double w = (maxValue - minValue) / bincount;
  int finalArr[bincount]; // an array that holds the final bin occurences

  // Initilization of finalArr to 0 for all elements
  for(int i = 0; i < bincount; i++){
    finalArr[i] = 0;
  }

  // Below loop reads the B batches of linked list if any B batch is supplied.
  // It controls the existence by waiting first a thread to supply a batch if
  // no batch finish signal is supplied from a thread. It loop while not all
  // threads finished. Because while threads are still working it exchanges data
  // when a B batch is supplied but when all threads are finished, the all remaining
  // datas can be read from the list by a simple loop.
  while(exitNum < N){
      int counter = 0;

      // Mutex for uninterrupted batch read.
      // Critical section begins
      pthread_mutex_lock(&globMutex1);

      // If no signal is supplied, it waits a batch to be supplied.
      while(signalCount == 0){
	pthread_cond_wait(&batchCond, &globMutex1);
      }
      signalCount--; // batch finish count decrement in each batch gathered.

      // Below loop continues until a batch is finished or the list becomes empty.
      while(list != NULL && counter < batch){
	double temp = pop(&list);
	double result = floor((temp - minValue) / w);
	int temp2 = (int)result;

	// Test if the result is the maxvalue and if it is the maxvalue
	// include into the last bin else use the normal bincount
	if(temp2 == bincount){
	  finalArr[bincount-1]++;
	  counter++;
	}
	else{
	  finalArr[temp2]++;
	  counter++;
	}
      }
      pthread_mutex_unlock(&globMutex1);
      // Critical section ends
    }

  // If all threads finish before main function to retrieve all batches concurrently,
  // below loop takes the rest of the list and and increments respective binnumbers.
  if(exitNum == N){
    while(list != NULL){
      double temp = pop(&list);
      double result = floor((temp - minValue) / w);
      int temp2 = (int)result;

      // Test if the result is the maxvalue and if it is the maxvalue
      // include into the last bin else use the normal bincount
      if(temp2 == bincount){
	finalArr[bincount-1]++;
      }
      else{
	finalArr[temp2]++;
      }
    }
  }
  
  // This part writes the result into the outfile
  FILE *out = fopen(outFile, "w");
  for(int i = 0; i < bincount; i++){
    fprintf(out, "%d: %d\n", i+1, finalArr[i]); 
  }

  fclose(out); // closing the outfile
}
