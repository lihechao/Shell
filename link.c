#include "global.h"

int getSimpleCommandLength(SimpleCommand *commandList)
{
	SimpleCommand *current=commandList;
	int length = 0;
	while(current!=NULL)
	{
		length++;
		current = current->next; 
	}
	return length;
}

void freeCommand(SimpleCommand *head)
{
	SimpleCommand *current;
	int i;
	while(head!=NULL)
	{
		current = head;
		head = head->next;
		for(i = 0; current->args[i]!=NULL; i++) free(current->args[i]);
		free(current->inputFile);
		free(current->outputFile);
		free(current);
	}
	free(newSimpleCommand);
}

void printCommand(SimpleCommand *head)
{
	SimpleCommand *current;
	int i;
	current = head;
	while(current!=NULL)
	{
		printf("command:\t");
		for(i = 0; current->args[i]!=NULL; i++) printf("%s ",current->args[i]);
		printf("\n");
		if(current->inputFile!=NULL) printf("inputFile:\t%s\n",current->inputFile);
		if(current->outputFile!=NULL) printf("outputFile:\t%s\n",current->outputFile);
		printf("\n");
		current = current->next;
	}
}
