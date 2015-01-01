#include "global.h"

void getSimpleCommand()
{
	if(currentSimpleCommand == NULL) simpleCommandList = newSimpleCommand;
	if(currentSimpleCommand!=NULL)
		currentSimpleCommand->next = newSimpleCommand;
	currentSimpleCommand = newSimpleCommand;
	newSimpleCommand = (SimpleCommand*)malloc(sizeof(SimpleCommand));
	newSimpleCommand->args = NULL;
	newSimpleCommand->inputFile = NULL;
	newSimpleCommand->outputFile = NULL;
	newSimpleCommand->next = NULL;	
}

void getArgs( int argcs, char argvs[][COMMAND_MAX_LENGTH])
{
	int i,length;

	newSimpleCommand->args = (char**)malloc(sizeof(char*)*(argcs+1));
	newSimpleCommand->args[argcs] = NULL;

	for(i = 0; i < argcs; i++)
	{
		length = strlen(argvs[i]);
		newSimpleCommand->args[i] = (char*)malloc(sizeof(char)*(length+1));
		strcpy(newSimpleCommand->args[i], argvs[i]);
	}
}

void getInputRedirect(char *argvs)
{
	int length = strlen(argvs);
	newSimpleCommand->inputFile = (char*)malloc(sizeof(char)*(length+1));
	strcpy(newSimpleCommand->inputFile,argvs);
}

void getOutputRedirect(char *argvs)
{
	int length = strlen(argvs);
	newSimpleCommand->outputFile = (char*)malloc(sizeof(char)*(length+1));
	strcpy(newSimpleCommand->outputFile,argvs);
}
