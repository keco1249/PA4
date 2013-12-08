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
int getDirectoryFiles(List **fileList, char *name, char *ip, char *port);

int fileToBuffer(char *fileName, char **buffer);

int handleCommand(char *command, char *file, List *fileList);

int listToBuffer(List * fileList, char *buffer);

int registerClientName(char *name, char* ip, char* port, int sock);

int registerFiles(int sock, List * fileList);

int requestToBuffer(int size, char *name, char *buffer);


int main (int argc, char * argv[])
{ 
  List * fileList = emptylist();
  char* myPort = "9090";
  char* myIP = "127.0.0.1";
  int getSocket;
  int getPort;
  pid_t pid;
  int status;
  
  if(argc < 4){
    printf("<Client Name> <Server IP> <Server Port>\n");
    exit(1);
  }
  
  if(getDirectoryFiles(&fileList, argv[1], myIP, myPort)){
    printf("error getting files");
  }
  display(fileList);
  FileInfo searchInfo;
  if(search("linkedlist.c", fileList, &searchInfo)){
    printf("name: %s\n", searchInfo.name);
    printf("port: %s\n", searchInfo.port);
  }
  else{
    printf("file not found");
  }
  /*char *fileBuffer;  
  fileToBuffer(33, "file", &fileBuffer);
  printf("File Buffer: %s\n", fileBuffer);
  FILE *newFile = NULL;
  bufferToFile(fileBuffer, newFile, "file2", 0);*/
  
  struct sockaddr_in serv_addr;
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = 0;//htons(9092);
  
  getSocket = 0;
  getSocket = socket(AF_INET, SOCK_STREAM, 0);       
  //set SO_REUSEADDR so that socket immediate unbinds from port after closed
  int optVal = 1;
  setsockopt(getSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
  if(bind(getSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
    printf("Bind Failed: %s\n", strerror(errno));
    return 1;
  } 
  if(listen(getSocket, 3)){
    printf("Listen Failed: %s\n", strerror(errno));
    return 1;
  }
  
  memset(&serv_addr, '0', sizeof(serv_addr));
  socklen_t len = sizeof(serv_addr);
  getsockname(getSocket, (struct sockaddr*)&serv_addr, &len);
  getPort = ntohs(serv_addr.sin_port);
  printf("Listening on port: %d\n", getPort);

  pid = fork();
  if(pid >= 0){
    if(pid == 0){
      //child
      char buffer[64];
      while(1){
	int connfd = accept(getSocket, (struct sockaddr*)NULL, NULL);
	int received = 0;
	 while(!received){
	   printf("child receive\n");
	   if(recv(connfd, buffer, 64, 0) < 0){
	     perror("recv error\n");
	   }
	   else{
	     // handle file request
	     char * fileRequestBuffer;
	     printf("Received request: %s\n", buffer); 
	     received = 1;
	     int size = strlen(buffer);
	     printf("request size: %d\n", size);
	     fileRequestBuffer = malloc(size+1);
	     strncpy(fileRequestBuffer, buffer, size);
	     fileRequestBuffer[size+1] = '\0';
	     printf("Request: %s\n", fileRequestBuffer);
	     FileInfo info;
	     if(search(fileRequestBuffer, fileList, &info)){
	       printf("File found\n");
	       char *fileBuffer;
	       fileToBuffer(fileRequestBuffer, &fileBuffer);
	       int fileSize = strlen(fileBuffer);
	       printf("Sending %d bytes in message: %s\n", fileSize, fileBuffer);
	       send(connfd, fileBuffer, fileSize, 0);
	     }
	     else{
	       printf("File not found\n");
	     }
	   }
	 }
	 printf("Received loop done\n");
	 close(connfd);
      }  
    }
    else{
      //parent
      // init server addr with addr and port provided in argv
      int socketfd;
      struct sockaddr_in serv_addr;
      memset(&serv_addr, '0', sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
      serv_addr.sin_port = htons(atoi(argv[3]));
      
      // create socketfd and set SO_REUSEADDR
      socketfd = socket(AF_INET, SOCK_STREAM, 0);
      int optVal = 1;
      setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
      
      /*
      // attempt to connect to server
      if(connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
      // connection failed
      printf("Connect Failed: %s\n", strerror(errno));
      return 1;
      }
      else {
      // successful connection
      // register client
      if(registerClientName(argv[1], myIP, myPort, socketfd)){
      printf("Client Name Already Registered: %s\n", argv[1]);
      printf("Exiting\n");
      exit(1);
      }
      // send client files
      if(registerFiles(socketfd, fileList)){
      printf("Failed to register files\n");
      exit(1);
      }
      }
      */
        char command[MAXBUFSIZE];
	// wait for commands 
	while(1){
	  bzero(command, sizeof(command));
	  //Read from command line to String(s)
	  fgets(command, sizeof(command), stdin);
      
	  if(strlen(command) > 0) {
	    // command line input is handled inside of handleCommand function
	    // it returns 1 on an exit, breaks the while loop and exits
	    
	    if(handleCommand(command, NULL, fileList)){// !!!! fileList should be replaced by master list !!!!!
	      break;
	    }
	  }
	}
    }
  }
  // ===========================================
  // Fork process to listen to get requests here
  // ===========================================


  
  exit(0);
}

//Read char buffer to File descriptor
int bufferToFile(char *buffer, FILE *file, char *fileName, int *fileSize){
  char *fileBuffer;
  fileBuffer = malloc(*fileSize+1);
  printf("malloc done\n");
  printf("opening file: %s\n", fileName);
  file = fopen(fileName, "wb");
  if(file == NULL){
    printf("error opening file\n");
  }
  printf("file opened\n");
  fwrite(buffer, *fileSize, 1, file);
  
  fclose(file);
  return 0;
}

// Read char buffer into a linkedlist
int bufferToList(char *buffer, struct LinkedList *fileList){
  /*BOTH*/
  return 0;
}

// Collect file information from directory
int getDirectoryFiles(List **fileList, char *name, char *ip, char *port){
  
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

	FileInfo fileInfo = {fileName, fileStats.st_size, name, ip, port};
	add(fileInfo, *fileList);
	
      }
    }
  } while (dp != NULL);

  return 0;
}

// Read file into buffer
// Structure |fileSize|File|
int fileToBuffer(char *fileName, char **buffer){
  char sizeBuffer[MAXINTSIZE+1];
  FILE *fp;
  int size;
  fp = fopen(fileName, "r");
  if(!fp)
    printf("Error opening file\n");

  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  char *fileBuffer = malloc(size+1);
  
  *buffer = malloc(size+1);
  
  //sprintf(sizeBuffer, "%08x", size);
  //strcat(*buffer, sizeBuffer);
  size_t newLen = fread(fileBuffer, sizeof(char), (size+1), fp);
  if (newLen == 0) {
    fputs("Error reading file", stderr);
    return 1;
  }
  strcat(*buffer, fileBuffer);

  return 0;
}

// Handle command line input return 1 on exit or failure
int handleCommand(char *command, char *file, List *fileList){
  char buffer[4600];
  if(strstr(command, "exit") != NULL) {
    printf("recived exit comman\n");
    return 1;
  }
  else if(strstr(command, "ls") != NULL){
    /* LUKE */
    // Get updated master list
    // print out updated master list
    printf("recived ls command\n");
    return 0;
  }
  else if(strstr(command, "get") != NULL){
    /* KEVIN */
    // lookup file address from masterlist
    // create connection with client
    // send file request and retrieve file
    // close connection
    char commandBuffer[4];
    char nameBuffer[56];
    char portBuffer[5];
    char ipBuffer[10];
    sscanf(command,"%s %s %s %s", commandBuffer, nameBuffer, portBuffer, ipBuffer);
    char *requestBuffer;
    //int nameSize = strlen(command)-5;
    int nameSize = strlen(nameBuffer);
    printf("name size: %d\n", nameSize);
    requestBuffer = malloc(nameSize);
    //strncpy(requestBuffer, command+4, nameSize);
    printf("request: %s\n", nameBuffer);
    FileInfo info;
    //if(search(nameBuffer, fileList, &info)){
    //printf("file found\n");
    int parentSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in client_addr;
    memset(&client_addr, '0', sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int port = (int)strtol(portBuffer, (char **)NULL, 10);
    printf("Port: %d\n", port);
    client_addr.sin_port = htons(port); 
    int optVal = 1;
    setsockopt(parentSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
    int connected = 0;
    while(!connected){
      //blindly attempt to connect
      if(connect(parentSocket, (struct sockaddr *)&client_addr, sizeof(client_addr))){
	printf("error connecting\n");
      }
      else{
	printf("Connected\n");
	connected = 1;
      }
      //}
      
      if(send(parentSocket, nameBuffer, nameSize, 0) >= 0){
	printf("message sent\n");
	printf("Request buffer size: %d\n", nameSize);
	printf("Request Buffer Sent: %s\n", requestBuffer);
      }
      
      if(recv(parentSocket, buffer, 4600, 0) < 0){
	perror("recv error\n");
      }
      else{
	int size = strlen(buffer);
	printf("File Size: %d\n", size);
	printf("File name: %s\n", nameBuffer);
	FILE *fd = NULL;
	bufferToFile(buffer, fd, nameBuffer, &size);
      }
      
    }
    //else{
    //printf("File not found\n");
    //}
    return 0;
  }
  printf("Error: Unrecognized Command\n");
  return 0;
}

// Convert linkedlist to buffer for sending over connection
int listToBuffer(List * fileList, char *buffer){
  bzero(buffer, sizeof(buffer));

  if(fileList->head != NULL) {
    Node * currNode = fileList->head;
    strcat(buffer, "<");
    strcat(buffer, currNode->data.name);
    strcat(buffer, ",");
    sprintf(buffer, "%s%d", buffer, currNode->data.size);
    strcat(buffer, ",");
    strcat(buffer, currNode->data.owner);
    strcat(buffer, ",");
    strcat(buffer, currNode->data.ip);
    strcat(buffer, ",");
    strcat(buffer, currNode->data.port);
    strcat(buffer, ">");
  
    while(currNode->next != NULL) {
      currNode = currNode->next;
      strcat(buffer, "<");
      strcat(buffer, currNode->data.name);
      strcat(buffer, ",");
      sprintf(buffer, "%s%d", buffer, currNode->data.size);
      strcat(buffer, ",");
      strcat(buffer, currNode->data.owner);
      strcat(buffer, ",");
      strcat(buffer, currNode->data.ip);
      strcat(buffer, ",");
      strcat(buffer, currNode->data.port);
      strcat(buffer, ">");
    }

    if(strlen(buffer) > 0) return 0;
    else return 1;
  }
  else {
    return 1;
  }
}

// Register client name with server
int registerClientName(char *name, char *ip, char *port, int sock){
  // If file name already exists in master list return 1
  // else add files to list, register name, and return 0
  
  char buffer[MAXBUFSIZE];
  bzero(buffer, sizeof(buffer));

  strcat(buffer, name);
  strcat(buffer, "|");
  strcat(buffer, ip);
  strcat(buffer, "|");
  strcat(buffer, port);

  // send name
  if(send(sock, buffer, sizeof(buffer), 0) > 0) {
    // wait for response
    bzero(buffer, sizeof(buffer));
    if(recv(sock, buffer, sizeof(buffer), 0) > 0) {
      // check response for success or failure
      if(strcmp(buffer, "success") == 0) {
	return 0;
      }
      if(strcmp(buffer, "failure") == 0) {
	return 1;
      }
      else {
	return 1;
      }
    }
    else {
      printf("Recive after name Send Failed: %s\n", strerror(errno));
      return 1;
    }
  }
  else {
    printf("Name Send Failed: %s\n", strerror(errno));
    return 1;
  }

  return 1;
}

// Register client files 
int registerFiles(int sock, List * fileList){
  char buffer[MAXBUFSIZE];

  if(listToBuffer(fileList, buffer)) {
    printf("Failed to convert file list to buffer\n");
    exit(1);
  }

  // send buffer to server
  if(send(sock, buffer, sizeof(buffer), 0) > 0) {
    // wait for response
    bzero(buffer, sizeof(buffer));
    if(recv(sock, buffer, sizeof(buffer), 0) > 0) {
      // check response for success or failure
      if(strcmp(buffer, "success") == 0) {
	return 0;
      }
      if(strcmp(buffer, "failure") == 0) {
	return 1;
      }
      else {
	return 1;
      }
    }
    else {
      printf("Recive after file list Send Failed: %s\n", strerror(errno));
      return 1;
    }
  }
  else {
    printf("File List Send Failed: %s\n", strerror(errno));
    return 1;
  }

  return 1;
}

// Create file request to buffer - MIGHT BE UNNECESSARY 
int requestToBuffer(int size, char *name, char *buffer){
  // map size and name to buffer
  return 0;
}
