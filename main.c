#include "global.h"
int goon, fgPid;

void ctrl_Z_predo()
{
		Job *now = NULL;
		if(ingnore == CTRL_Z)
		{
			 now = head;
			while (now != NULL && now->pid != fgPid)
				now = now->next;
			if(now == NULL)//未找到前台作业，则根据fgPid添加前台作业
			{
				now = addJob(fgPid);
			}
			//修改前台作业的状态及相应的命令格式，并打印提示信息
			strcpy(now->state, STOPPED);
			//now->cmd[strlen(now->cmd)] = '&';
			now->cmd[strlen(now->cmd) + 1] = '\0';
			printf("\n[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
			fgPid = 0;
			ingnore = 0;
		}		
}

int main(int argc,char** argv)
{
	char c;
	int i;
	
	goon = fgPid = 0;
	history.end=-1;
    history.start=0;
	init();
	//ingnore = 0;
	while(1){
		
		ctrl_Z_predo();
		simpleCommandList = NULL, currentSimpleCommand = NULL;
		newSimpleCommand = (SimpleCommand*)malloc(sizeof(SimpleCommand));
		newSimpleCommand->args = NULL;
		newSimpleCommand->inputFile = NULL;
		newSimpleCommand->outputFile = NULL;
		newSimpleCommand->next = NULL;

	

		printf("user-sh@%s>",get_current_dir_name());

		i = 0;
		//首先判断是否是可打印字符
		c=getchar();
		while(!(isprint(c) || isspace(c)))  { c = getchar(); }
		//读入输入字符串
		while(c != '\n') { inputBuff[i++] = c;c=getchar();}
		//调整输入末尾值看是否为空串
		for(i = i-1; i >= 0 && isspace(inputBuff[i]); i--) ;
		inputBuff[++i] = '\0';
		len = i;
		offset = 0;

		init();
		
		
        if(len > 0) { 
			addHistory(inputBuff); 
		
			
			yyparse();

			if(rightCommand && commandExists(simpleCommandList)){ execute(); commandDone = 1;}

			freeCommand(simpleCommandList);
		
		}

	}
	return (EXIT_SUCCESS);
}
