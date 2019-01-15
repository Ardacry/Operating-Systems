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

int main(int argc, char *argv[]){

  FILE *fp = fopen("CalculationResults", "r");
  
  int fcfs; // it holds scanned values from the file
  int sstf;
  int scan;
  int cscan;
  int look;
  int clook;
  int fileEnd = 0; // character check for end of the file

  int fcfstot = 0;
  int sstftot = 0;
  int scantot = 0;
  int cscantot = 0;
  int looktot = 0;
  int clooktot = 0;

  int fcfsArr[100];
  int sstfArr[100];
  int scanArr[100];
  int cscanArr[100];
  int lookArr[100];
  int clookArr[100];

  int counter = 0;
  // This loop reads each value in the line and calculates
  // which bin it belongs to by rounding it to its floor
  // integer and increments that bin representation in the array
  // by one.
  do{
    if(fscanf(fp, "%d", &fcfs) != EOF){

      fscanf(fp, "%d", &sstf);
      fscanf(fp, "%d", &scan);
      fscanf(fp, "%d", &cscan);
      fscanf(fp, "%d", &look);
      fscanf(fp, "%d", &clook);

      fcfstot += fcfs;
      sstftot += sstf;
      scantot += scan;
      cscantot += cscan;
      looktot += look;
      clooktot += clook;

      fcfsArr[counter] = fcfs;
      sstfArr[counter] = sstf;
      scanArr[counter] = scan;
      cscanArr[counter] = cscan;
      lookArr[counter] = look;
      clookArr[counter] = clook;

      counter++;
      
    }

    // fileEnd becomes one if EOF is encountered
    else{
      fileEnd = 1;
    }
  }while(fileEnd != 1);

  fclose(fp);

  double fcfsMean = (double)fcfstot / 100;
  double sstfMean = (double)sstftot / 100;
  double scanMean = (double)scantot / 100;
  double cscanMean = (double)cscantot / 100;
  double lookMean = (double)looktot / 100;
  double clookMean = (double)clooktot / 100;
  
  printf("FCFS mean is : %f\n", fcfsMean);
  printf("SSTF mean is : %f\n", sstfMean);
  printf("SCAN mean is : %f\n", scanMean);
  printf("C-SCAN mean is : %f\n", cscanMean);
  printf("LOOK mean is : %f\n", lookMean);
  printf("C-LOOK mean is : %f\n", clookMean);

  double sum = 0;

  for (int i = 0; i < 100; i++)
    sum = sum + pow((fcfsArr[i] - fcfsMean), 2);

  double sd = sqrt(sum / 100);
  printf("FCFS standard deviation : %f\n", sd);

  sum = 0;
  for (int i = 0; i < 100; i++)
    sum = sum + pow((sstfArr[i] - sstfMean), 2);

  sd = sqrt(sum / 100);
  printf("SSTF standard deviation : %f\n", sd);

  sum = 0;
  for (int i = 0; i < 100; i++)
    sum = sum + pow((scanArr[i] - scanMean), 2);

  sd = sqrt(sum / 100);
  printf("SCAN standard deviation : %f\n", sd);

  sum = 0;
  for (int i = 0; i < 100; i++)
    sum = sum + pow((cscanArr[i] - cscanMean), 2);

  sd = sqrt(sum / 100);
  printf("CSCAN standard deviation : %f\n", sd);

  sum = 0;
  for (int i = 0; i < 100; i++)
    sum = sum + pow((lookArr[i] - lookMean), 2);

  sd = sqrt(sum / 100);
  printf("LOOK standard deviation : %f\n", sd);

  sum = 0;
  for (int i = 0; i < 100; i++)
    sum = sum + pow((clookArr[i] - clookMean), 2);

  sd = sqrt(sum / 100);
  printf("C-LOOK standard deviation : %f\n", sd);

}

