%{
	#include "global.h"

	int yylex();
	void yyerror();
	

	int offset,len,commandDone,rightCommand;
	int isBack;
	char inputBuff[COMMAND_MAX_LENGTH];
	char strings[10][COMMAND_MAX_LENGTH];
	int stringsCount;
	int rightCommand;
	SimpleCommand *simpleCommandList = NULL;
	SimpleCommand *currentSimpleCommand = NULL;
	SimpleCommand *newSimpleCommand = NULL;
%}

%token STRING

/***********************************************************************/
/*                            规则说明				       */
/***********************************************************************/

%%
	line		: /*empty*/
			 | command ;
	command		: fgCommand{ isBack = 0; }
			 | fgCommand '&'{ isBack = 1; };//判断是否是后台程序
	fgCommand	: simpleCmd 
			 | pipeCmd ;
	simpleCmd	: progInvocation inputRedirect outputRedirect { getSimpleCommand(); };//使用链表数据结构获取命令结构
	pipeCmd		: simpleCmd '|' simpleCmd
			 | simpleCmd '|' pipeCmd;
	progInvocation	: STRING args {  getArgs(stringsCount , strings); stringsCount = 0; } ;
	inputRedirect	: /*empty*/
			 | '<' STRING { getInputRedirect(strings[0]); stringsCount = 0; };
	outputRedirect	: /*empty*/
			 | '>' STRING { getOutputRedirect(strings[0]); stringsCount = 0;};
	args		: /*empty*/
			 | STRING args ;
%%

/*********************************************************************/
/*			词法分析函数				     */
/*********************************************************************/

int yylex()
{
	int flag,i;
	char c;
	
	//跳过空格
	while(offset<len && isspace(inputBuff[offset])) offset++;
	
	flag = 0;
	i = 0;
	while(offset<len)
	{
		c = inputBuff[offset];
		
		if(isspace(c)) 
		{ 
			strings[stringsCount][i] = '\0';
			stringsCount++;
			offset++; 
			return STRING;
		}
		
		if(c == '<' || c == '>' || c== '&' || c=='|')
		{
			if(flag == 1)
			{
			  flag = 0; 
			  strings[stringsCount][i] = '\0';
			  stringsCount++;
			  return STRING;
			}
			offset++;
			return c;
		}

		flag = 1;
		strings[stringsCount][i++]=c;
		offset++;
	}

	if(flag == 1) 
	{
		strings[stringsCount][i] = '\0';
		stringsCount++;
		return STRING;
	}
	else return 0;
}

void yyerror()
{
	rightCommand = 0;
	printf("error : 您输入的命令不正确，请确认后再重新输入\n");
}



