#include "global.h"

#define MAXCOMMAND 255
void ctrl_Z();
void setGoon();
void execOuterCmd(SimpleCommand *cmd);
void init();
void execInnerCmd(SimpleCommand *cmd);
void bg_exec(int pid);
void fg_exec(int pid);
Job *addJob(pid_t pid);
int goon;
pid_t fgpid;
Job *head=NULL;
History history;
pid_t shellpid;
int ingnore;
pid_t last;
pid_t waittemp;

void execute()
{

	SimpleCommand *current = simpleCommandList;
	int commandCount = getSimpleCommandLength(simpleCommandList);
	int i = 0, j =0;
	int sleepAlittle;
	int pid[MAXCOMMAND],pipe_fd[MAXCOMMAND][2];
	int newpgrppid;
	shellpid = getpid();
	//这是一个简单命令
	if(commandCount == 1)
	{
		if(isInnerCommand(current->args[0]) ) { execInnerCmd(current); }
		else
		{

			if((newpgrppid = fork()) < 0) { perror("Fork failed"); return ;}
			else fgPid = newpgrppid;
			if(!newpgrppid)
			{
				
				if(!isBack) fgPid = getpid();
				setpgid(0, 0);
				if(!isBack) 
				{
					tcsetpgrp(0, getpid());
				}
				
				execOuterCmd(current);
				exit(EXIT_SUCCESS);
				
			}
			
			setpgid(newpgrppid, newpgrppid);	
			if(!isBack) 
			{
				tcsetpgrp(0, newpgrppid);
			}

			if(isBack)//is back cmd
			{
				fgPid=0;
				addJob(newpgrppid);
				//等待子进程注册在添加上上面的addJob命令后删掉下面的代码
				for(sleepAlittle = 0; sleepAlittle < 1000000; sleepAlittle++) ;
				kill(-newpgrppid,SIGUSR1);//向子进程发消息
				//等待子进程输出
				signal(SIGUSR1,setGoon);
				while(goon==0) ;
				goon=0;
			}
			else
			{
				fgPid=newpgrppid;
				waitpid(newpgrppid,NULL,0);
				if(!isBack) 
				{
					setpgid(0,0);					
					tcsetpgrp(0,getpid());	
				}
			}
		}
	}
	//这是一个管道命令
	else
	{
		
		for(i = 0; i < commandCount && current != NULL; i++)
		{
			//创建管道
			if(i < commandCount -1)
			{

				if(pipe(pipe_fd[i]) < 0) { perror("Pipe failed");return ;}
			}
			//为命令创建进程
			if((pid[i] = fork()) < 0) { perror("Fork failed"); return ;}
			if(i == 0)
			{
				if(!pid[0])
				{
					if(!isBack) fgPid = getpid();
					setpgid(0, 0);
					if(!isBack) 
					{
						tcsetpgrp(0, getpid());
					}
				}
				else
				{
					setpgid(pid[0], pid[0]);	
					if(!isBack) 
					{
						tcsetpgrp(0, pid[0]);
					}
				}
			}
			if(!pid[i])//子进程
			{
				if(i > 0)
				{
					setpgid(pid[i],pid[0]);
					close (pipe_fd[i-1][1]);
					dup2 (pipe_fd[i-1][0], 0);
					close (pipe_fd[i-1][0]);
				}

				if(i < commandCount - 1)
				{
					close (pipe_fd[i][0]);
					dup2 (pipe_fd[i][1], 1);
					close (pipe_fd[i][1]);
				}


				if(isInnerCommand(current->args[0])) { execInnerCmd(current); }
				else { execOuterCmd(current); }

				return ;
			}

			if(i > 0)
			{
				close (pipe_fd[i-1][0]);
				close (pipe_fd[i-1][1]);
			}

			current = current->next;

		}

	
		if(isBack)//is back cmd
		{
			fgPid=0;
			addJob(pid[0]);
			//等待子进程注册在添加上上面的addJob命令后删掉下面的代码
			for(sleepAlittle = 0; sleepAlittle < 1000000; sleepAlittle++) ;
			kill(pid[i-1],SIGUSR1);//向子进程发消息
			//等待子进程输出
			signal(SIGUSR1,setGoon);
			while(goon==0) ;
			goon=0;
		}
		else
		{
			fgPid=pid[0];
			waitpid(pid[i-1],NULL,0);
			if(!isBack) 
			{
				setpgid(0,0);
				tcsetpgrp(0,getpid());
			}
		}
	}
}

int commandExists(SimpleCommand *commandList)
{
	SimpleCommand *current = commandList;
	char *commandFile;
	char *envPath[] = { "/bin/", "/usr/bin/", "/usr/local/bin/", "/sbin/", NULL};
	char *innerCommand[] = { "exit", "history", "jobs", "cd", "fg", "bg", NULL};
	char cmdBuff[COMMAND_MAX_LENGTH];
	int foundPath = 0, i = 0;
	while(current != NULL)
	{
		commandFile = current->args[0];
		foundPath = 0;

		//搜索是否是全路径命令
		if((commandFile[0] == '/' || commandFile[0] == '.') && access(commandFile,F_OK) == 0) foundPath = 1;
		else
		{
			//搜索是否是内部命令
			for(i = 0; innerCommand[i] != NULL; i++)
			{
				if(strcmp(commandFile, innerCommand[i]) == 0) { foundPath = 1; break;}
			}
			//搜索是否是外部命令
			if(!foundPath){
				for(i = 0; envPath[i] != NULL; i++)
				{
					strcpy(cmdBuff, envPath[i]);
					strcat(cmdBuff, commandFile);
					if(access(cmdBuff, F_OK) == 0){
						foundPath = 1;
						current->args[0] = (char*)malloc(sizeof(char)*(strlen(cmdBuff)+1));
						strcpy(current->args[0], cmdBuff);
						free(commandFile);
						break;
					}
				}
			}
		}
		if(!foundPath && commandFile!=NULL) { printf("对不起，找不到命令%s的路径，请您确认后重新输入。\n", commandFile); return 0;}
		current = current->next;
	}
	return 1;
}

int isInnerCommand(char *command)
{
	char *innerCommand[] = { "exit", "history", "jobs", "cd", "fg", "bg", NULL};
	int i = 0;
	for(i = 0; innerCommand[i] != NULL; i++)
	{
		if(strcmp(command, innerCommand[i]) == 0) return 1;
	}
	return 0;
}


void setGoon(){
    goon=1;
}


int str2Pid(char *str,int start,int end){
    int i,j;
    char chs[20];
    
    for(i=start,j=0;i<end;i++,j++){
        if(str[i]<'0'||str[i]>'9')
           return -1;
         else chs[j]=str[i];
    }
    chs[j]='\0';
    return atoi(chs);
}


void execOuterCmd(SimpleCommand *cmd){//执行外部命令
	int pipeIn,pipeOut;

	if(cmd->inputFile!=NULL){//存在输入重定向
		if((pipeIn=open(cmd->inputFile,O_RDONLY,S_IRUSR|S_IWUSR))==-1){
		    printf("can't open the file %s\n",cmd->inputFile);
		    return ;

		}
		if(dup2(pipeIn,0)==-1){
		    printf("重定向错误\n");
		    return ;
		}
	}

	if(cmd->outputFile!=NULL)
	{//存在输出重定向
		if((pipeOut=open(cmd->outputFile,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))==-1){
		  printf("can't open the file %s\n",cmd->outputFile);
		    return ;
		}

		if(dup2(pipeOut,1)==-1){
		    printf("重定向错误\n");
		    return ;
		}
	}
	
	if(isBack && cmd->next == NULL){//如果是后台命令并且是最后一条命令
		signal(SIGUSR1,setGoon);
		while(goon==0) ;
		goon=0;
		kill(shellpid,SIGUSR1);
	}

	if(execvp(cmd->args[0],cmd->args)<0){
		printf("execv faileed\n");
		exit(EXIT_SUCCESS);
	}
}


void execInnerCmd(SimpleCommand *cmd){
    int i,pid;
    char *temp;
    Job *now=NULL;

    if(strcmp(cmd->args[0],"exit")==0){//exit
        exit(EXIT_SUCCESS);
    }

    else if(strcmp(cmd->args[0],"history")==0){//history
        if(history.end==-1){
            printf("还没有执行过任何命令\n");
            return;
        }
        i=history.start;
         do{
             printf("%s \n",history.command[i]);
             i=(i+1)%HISTORY_LENGTH;
         }while(i!=(history.end+1)%HISTORY_LENGTH);
         return;

    }

    else if(strcmp(cmd->args[0],"jobs")==0){//jobs
        if(head==NULL){
            printf("现在没有任何作业\n");
            return;
        }

        else{
            printf("index\tpid\tstate\t\tcmd\n");
            for(i=1,now=head;now!=NULL;i++,now=now->next)
                printf("%d\t%d\t%s\t\t%s\n",i,now->pid,now->state,now->cmd);
            return;
        }

    }
    else if(strcmp(cmd->args[0],"fg")==0){//fg
        temp=cmd->args[1];
        if(temp!=NULL&&temp[0]=='%'){
            if(isdigit(temp[1]))
                pid=str2Pid(temp,1,strlen(temp));
             else {
                 pid= str2Pid(cmd->args[2],0,strlen(cmd->args[2]));
             }
            if(pid!=-1){
                fg_exec(pid);
                return;
            }
            else {
                printf("fg参数格式不正确\n");
                return;
            }
        }
    }
    else if(strcmp(cmd->args[0],"bg")==0){//bg
        temp=cmd->args[1];
        if(temp!=NULL&&temp[0]=='%'){
            if(isdigit(temp[1]))
                pid=str2Pid(temp,1,strlen(temp));
             else {
                 pid= str2Pid(cmd->args[2],0,strlen(cmd->args[2]));
                   }
            if(pid!=-1){
                bg_exec(pid);
                return;
            }
            else {
                printf("bg参数格式不正确\n");
                return;
            }
        }


    }
    else if(strcmp(cmd->args[0],"cd")==0){
        temp=cmd->args[1];
        if(temp!=NULL){
			if(strcmp(temp,"~") == 0)
			{
				if(chdir(getenv("HOME"))<0){
		            printf("cd %s,error filename\n",temp);
		            return;
            	}
			}
			else
			{
		        if(chdir(temp)<0){
		            printf("cd %s,error filename\n",temp);
		            return;
		        }
			}
        }
		else
		{
			 if(chdir(getenv("HOME"))<0){
                printf("cd %s,error filename\n",temp);
                return;
            }
		}
    }

}

/*fg命令*/

void fg_exec(int pid)
{
    Job *now = NULL;
    int i;
    //SIGCHLD信号由此产生

    now = head;
    while(now !=NULL && now->pid != pid)
    {
        now = now->next;
    }
    if(now == NULL||strcmp(now->state,DONE)==0)//为找到作业
    {
        printf("pid为%d的作业不存在\n", pid);
        return;
    }
    //记录前台pid，修改对应作业状态
    fgPid = pid;//now->pid;
    strcpy(now->state, RUNNING);
	now->isback = 0;
    i = strlen(now->cmd) - 1;
    while(i >= 0 && now->cmd[i] != '&')
    i--;
    now->cmd[i] = '\0';
    
	tcsetpgrp(0, fgPid);
	ingnore = 0;
    kill(-fgPid, SIGCONT);//向对象作业发送SIGCONT信号，使其运行
	
	for(pid = 0 ; pid < 100000; pid++) ;

    waitpid(fgPid, NULL, 0);//父进程等待前台进程的运行
	setpgid(0,0);
	tcsetpgrp(0,getpid());
}


void bg_exec(int pid){
    Job *now=NULL;
    int i = 0;
	
    now=head;
    while(now != NULL&&now->pid != pid)
        now=now->next;
    if(now == NULL){
        printf("pid 为%d 的作业不存在\n",pid);
        return;
    }
	i = strlen(now->cmd) - 1;
	if(now->cmd[i] != '&')
	{
		i++;
		now->cmd[i] = '&';	
	}
	i++;
    now->cmd[i] = '\0';
	isBack = 1;
    strcpy(now->state,RUNNING);
	now->isback = 1;
    printf("[%d]\t%s\t\t%s\n",now->pid,now->state,now->cmd);
    kill(-now->pid,SIGCONT);
}

void addHistory (char *cmd)
{
    if(history.end == -1) //第一次使用command命令
    {
        history.end = 0;
        strcpy(history.command[history.end], cmd);
        return;
    }
    history.end = (history.end + 1)%HISTORY_LENGTH;//end向前一位
    strcpy(history.command[history.end], cmd);//将命令副本到end指向的数组中

    if(history.end == history.start)     //end和start指向同一位置
    {
        history.start = (history.start + 1)%HISTORY_LENGTH;//start向前移一位
    }

}

/*添加新的作业*/
Job *addJob(pid_t pid)
{
    Job *now = NULL, *last = NULL, *job = (Job*)malloc(sizeof(Job));
    int i = 0;

    job->pid = pid;
    strcpy(job->cmd, inputBuff);
    strcpy(job->state, RUNNING);
	job->isback = 1;
    job->next = NULL;

    if(head == NULL)
    {
        head = job;
    }
    else            //否则，根据pid将新的job插入道链表的合适的位置
    {
        now = head;
        while(now != NULL && now->pid < pid)
        {
            last = now;
            now = now->next;
        }
        last->next = job;
        job->next = now;
    }

    return job;
}


/*sigchld信号处理*/
void signalchld(int sig, siginfo_t *sip, void* noused)
{
    pid_t pid;
    Job *now = NULL, *last = NULL;
	int hasWait = 0;
	pid = sip->si_pid;
	//ctrl_Z发出的信号
	if(ingnore == CTRL_Z || sip->si_code == CLD_STOPPED)
	{
		ingnore = CTRL_Z;
		return ;
	}
	
	
	if(isBack){
		waitpid(pid,NULL,0);
		hasWait = 1;
	}
	
    now = head;
    while(now != NULL && now->pid < pid)
	{
        last = now;
        now = now->next;
    }

    if(now == NULL) { return ;}//作业不存在，则不仅行处理直接返回
   

    strcpy(now->state,DONE);
	
	if(!hasWait && now->isback == 1) { waitpid(pid,NULL,0); }
	//if(isBack || now->isback){
		//printf("[%d]\t%s\t\t%s\n",now->pid,now->state,now->cmd);
	//}
}

void ctrl_Z()
{
	ingnore = CTRL_Z;
	if(fgPid>0)kill(-fgPid,SIGSTOP);
   	return ;
}

void ctrl_C(){
	if(fgPid>0)
	{
		kill(-fgPid,SIGINT);
		printf("\n");
	}
	return ;
}

void init(){
    struct sigaction action;
	
	ingnore = 0;
	isBack = 0;

	commandDone =  0;
	rightCommand = 1;
	stringsCount = 0;

    action.sa_sigaction=signalchld;
    sigemptyset(&action.sa_mask);
    sigfillset(&action.sa_mask);
    action.sa_flags=SA_SIGINFO;
    sigaction(SIGCHLD,&action,NULL);
	signal(SIGTSTP,ctrl_Z);
	signal(SIGINT,ctrl_C);
	signal(SIGTTOU, SIG_IGN);
	
	removeJob();
	
}

void removeJob(){
    Job *q=head,*p= NULL;
    if(head==NULL)
       return;
    p=head->next;
	if(head->isback && strcmp(head->state,DONE)==0){
		printf("[%d]\t%s\t\t%s\n",head->pid,head->state,head->cmd);
	}
    while(p!=NULL){
        if(strcmp(p->state,DONE)==0){
			if(p->isback) printf("[%d]\t%s\t\t%s\n",p->pid,p->state,p->cmd);
            q->next=p->next;
            free(p);
            p=q->next;
        }
        else {
            q=p;
            p=p->next;
        }
    }
    if(strcmp(head->state,DONE)==0){
        q=head;
        head=head->next;
        free(q);
    }
}


