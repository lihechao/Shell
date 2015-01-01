/*
 * globle.h
 *
 *  Created on: 2012-3-7
 *      Author: zhangkai
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define COMMAND_MAX_LENGTH 256
#define HISTORY_LENGTH 10
#define STOPPED "stopped"
#define RUNNING "running"
#define DONE "done"
#define HAVE_CTRL_Z 1
#define FG 2
#define BG 6
#define CTRL_C 4
#define CTRL_Z 5

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/termios.h>

/*******************************
类型声明
********************************/
typedef struct SimpleCommand
{
	char **args;
	char *inputFile;
	char *outputFile;
	struct SimpleCommand* next;
}SimpleCommand;

typedef struct History
{
	int start;
	int end;
	char command[HISTORY_LENGTH][100];
}History;

typedef struct Job
{
    int pid;
    char cmd[100];
    char state[10];
	int isback;
    struct Job *next;
}Job;


/******************************
变量声明
*******************************/

/******************************/
/*    以下变量在yacc.y中声明   */
/******************************/
extern int commandDone;
extern int rightCommand;
extern int stringsCount;
extern int offset;
extern int len;
extern int isBack;
extern int goon;
extern pid_t fgPid;
extern char inputBuff[COMMAND_MAX_LENGTH];
extern SimpleCommand *simpleCommandList;
extern SimpleCommand *currentSimpleCommand;
extern SimpleCommand *newSimpleCommand;
extern Job *head;
/*********************************/

/*********************************

**********************************/
extern History  history;


/********************************
*********************************/

extern pid_t shellpid;
extern int ingnore;
/*******************************
 在哪个文件中声明
********************************/
//extern char inputBuff[COMMAND_MAX_LENGTH];

/*getCommand.c*/
void getSimpleCommand();
void getArgs( int argcs,char argvs[][COMMAND_MAX_LENGTH]);
void getInputRedirect(char *argvs);
void getOutputRedirect(char *argvs);

/*link.c*/
void freeCommand(SimpleCommand *head);
void freeCommand(SimpleCommand *head);
void printCommand(SimpleCommand *head);
int getSimpleCommandLength(SimpleCommand *commandList);

/*execute.c*/
void execute();
int commandExists(SimpleCommand *commandList);
int isInnerCommand(char *command);
Job *addJob(pid_t pid);
void removeJob();
void ctrl_Z();

#endif /* GLOBAL_H_ */
