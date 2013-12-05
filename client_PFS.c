// client_PFS.c
// Authors: Luke Connors and Kevin Colin
// linkedlist.h/linkedlist.c borrowed from https://github.com/skorks/c-linked-list
// and modified to include file data within each node

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//
#include "linkedlist.h"

#define MAXBUFSIZE 1024
#define MAXINTSIZE 8

struct LinkedList{
  //linkedList placeholder
};

int bufferToFile(char *buffer, FILE *file, char *fileName, int *fileSize);

int bufferToList(char *buffer, struct LinkedList *fileList);

//int getDirectoryFiles(struct LinkedList *fileList);
int getDirectoryFiles(List **fileList);

int fileToBuffer(int fileSize, char *fileName, char **buffer);

int handleCommand(char *command, char *file);

int listToBuffer(struct LinkedList fileList, char *buffer);

int registerClientName(char *name, int *sock);

int registerFiles(int sock);

int requestToBuffer(int size, char *name, char *buffer);


int main (int argc, char * argv[])
{ 
  int socket;

  List * list = emptylist();
  
  if(argc < 3){
    printf("<Client Name> <Server IP> <Server Port>\n");
    exit(1);
  }
  
  if(getDirectoryFiles(&list)){
    printf("error getting files");
  }
 
  char *fileBuffer;
  
  fileToBuffer(33, "file", &fileBuffer);
  
  printf("File Buffer: %s\n", fileBuffer);

  FILE *newFile = NULL;
  bufferToFile(fileBuffer, newFile, "file2", 0);
  

  // ===========================================
  // Fork process to listen to get requests here
  // ===========================================
  

  if(registerClientName(argv[1], &socket)){
    printf("Client Name Already Registered: %s\n", argv[1]);
    printf("Exiting\n");
    exit(1);
  }

  if(registerFiles(socket)){
    printf("Failed to register files\n");
    exit(1);
  }


  char *command;
  // wait for commands 
  while(1){
    //Read from command line to String(s)
    command = "";
    
    // command line input is handled inside of handleCommand function
    // it returns 1 on an exit, breaks the while loop and exits
    if(handleCommand(command, NULL)){
      break;
    }
  }
  
  exit(0);
}

//Read char buffer to File descriptor
int bufferToFile(char *buffer, FILE *file, char *fileName, int *fileSize){
  char sizeBuffer[MAXINTSIZE];
  char *fileBuffer;
  printf("Buffer: %s\n", buffer);
  strncpy(sizeBuffer, buffer, MAXINTSIZE);
  
  int size = strtol(sizeBuffer, NULL, 16);
  printf("Size of file: %d\n", size);
  fileBuffer = malloc(size);
  strncpy(fileBuffer, (buffer+MAXINTSIZE), size);
  
  file = fopen(fileName, "wb");
  fwrite(fileBuffer, size, 1, file);
  
  fclose(file);
  

  return 0;
}

// Read char buffer into a linkedlist
int bufferToList(char *buffer, struct LinkedList *fileList){
  /*BOTH*/
  return 0;
}

// Collect file information from directory
int getDirectoryFiles(List **fileList){
  
  DIR *dirp;
  struct dirent *dp;
  struct stat fileStats;
  char *fileName;
  *fileList = emptylist();

  if ((dirp = opendir(".")) == NULL) {
    perror("couldn't open '.'");
    return 1;
  }
  do {
    if ((dp = readdir(dirp)) != NULL) {
      fileName = dp->d_name;
      if((strcmp(fileName, ".") != 0) && (strcmp(fileName, "..") != 0)){
	stat(fileName, &fileStats);

	FileInfo fileInfo = {fileName, fileStats.st_size, "client_name", "127.0.0.1", "port"};
	add(fileInfo, *fileList);
	
      }
    }
  } while (dp != NULL);

  return 0;
}

// Read file into buffer
// Structure |fileSize|File|
int fileToBuffer(int fileSize, char *fileName, char **buffer){
  char sizeBuffer[MAXINTSIZE+1];
  char *fileBuffer = malloc(fileSize+1);
  FILE *fp;
  
  *buffer = malloc((MAXINTSIZE+1)+(fileSize+1));
  
  sprintf(sizeBuffer, "%08x", fileSize);
  strcat(*buffer, sizeBuffer);

  fp = fopen(fileName, "r");
  if(!fp)
    printf("Error opening file\n");
  size_t newLen = fread(fileBuffer, sizeof(char), (fileSize+1), fp);
  if (newLen == 0) {
    fputs("Error reading file", stderr);
    return 1;
  }
  strcat(*buffer, fileBuffer);

  return 0;
}

// Handle command line input return 1 on exit or failure
int handleCommand(char *command, char *file){
  if(strcmp(command, "exit"))
    return 1;
  else if(strcmp(command, "ls")){
    /* LUKE */
    // Get updated master list
    // print out updated master list
    return 0;
  }
  else if(strcmp(command, "get")){
    /* KEVIN */
    // lookup file address from masterlist
    // create connection with client
    // send file request and retrieve file
    // close connection
    return 0;
  }
  printf("Error: Unrecognized Command\n");
  return 0;
}

// Convert linkedlist to buffer for sending over connection
int listToBuffer(struct LinkedList fileList, char *buffer){
  return 0;
}

// Register client name with server
int registerClientName(char *name, int *sock){
  // If file name already exists in master list return 1
  // else add files to list, register name, and return 0
  printf("Client Name Registered: %s\n", name);
  return 0;
}

// Register client files 
int registerFiles(int sock){
  char *buffer;
  struct LinkedList list;

  // fileList = getDirectoryFiles(&fileList);
  // buffer = listToBuffer(fileList, buffer);
  /* LUKE */
  // send buffer to server

  return 0;
}

// Create file request to buffer - MIGHT BE UNNECESSARY 
int requestToBuffer(int size, char *name, char *buffer){
  // map size and name to buffer
  return 0;
}
