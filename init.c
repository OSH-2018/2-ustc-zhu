#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#define max_pipe 256
//int exec_cmd(char *args[128], int cnt);
int exec_pipe(char *args[128], int total_cnt_pipe, int curr_pipe, int num_args[max_pipe]);
int exec_cmd(char *args[128]);

int test(void){
	char a[30] = "pwd";
	char *b[3];
	b[0] = a;
	b[1] = NULL;
	exec_cmd(b);
	printf("flag!!!!!!\n");
}

int main() {
    while (1) {
    	/* 输入的命令行 */
   		 char cmd[256];
    	/* 命令行拆解成的各部分，以空指针结尾 */
   		 char *args[128];
    	/*count '|'*/
   		int total_cnt_pipe = 0;
    	int num_args[max_pipe];
    
   		 for (int i = 0; i < max_pipe; ++i)
    	{
    		num_args[i] = -1;
    	}

    	int save_stdin, save_stdout;
		save_stdin = dup(STDIN_FILENO);
    	save_stdout = dup(STDOUT_FILENO);
    	int pipe_error_flag = 0;
    	//test ' ' next to the '|'

        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */

       // printf("before %d\n", pipe_error_flag);

       // printf("cmd is %s\n", cmd);
        int i;
        for (i = 0; cmd[i] != '\n'; i++){
        	if (cmd[i] == '|'){
        		if (cmd[i-1] != ' ' || cmd[i+1] != ' '){
        			pipe_error_flag = 1;
        			//printf("%c %c %c\n", cmd[i-1], cmd[i], cmd[i+1]);
        			printf("please check the ' ' next to the '|'\n");
        			break;
        		}		
        	}
        }

      //  printf("after %d\n", pipe_error_flag);

      pipe_error_flag = 0;

        if(pipe_error_flag)
        	continue;
        //error

        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i];){
            if (*args[i] == ' ') {
                    *args[i] = '\0';
                    args[i]++;
                    continue;
                }
            //solve the problem: more than one ' ' between 2 args
            if (*args[i] == '|'){
            	total_cnt_pipe++;
            	num_args[total_cnt_pipe] = i;
            }
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++){
                if (*args[i+1] == ' ' || *args[i+1] == '=') {
                    *args[i+1] = '\0';
                    args[i+1]++;
                    break;
                }
            }
            i++;
        }
        args[i] = NULL;

//        test();

        //printf("args %s total_cnt_pipe %d\n", args[0], total_cnt_pipe);
			  

        if (total_cnt_pipe == 0)
        {
        	if(exec_cmd(args) == 1)
        		continue;
        	else{
        		printf("fail to run the command\n");
        		continue;
        	}
        }
        else{
        	/*pid_t pid = fork();
        	if (pid == 0)
        	{
			   	exec_pipe(args, total_cnt_pipe, num_args);
        	}*/
        	if(exec_pipe(args, total_cnt_pipe, total_cnt_pipe, num_args) == 1){
        		dup2(save_stdin,STDIN_FILENO);
        		dup2(save_stdout, STDOUT_FILENO);
        		continue;
        	}
        	else{
        		printf("fail to run the command\n");
        		continue;
        	}
        	/*else{
        		wait(NULL);
        		dup2(save_stdin,STDIN_FILENO);
        		dup2(save_stdout, STDOUT_FILENO);
        		continue;
        	}*/
        }
    }
}

int exec_cmd(char *args[128]){


	// printf("hahahahah\n");

	/* 没有输入命令 */
        if (!args[0])
            return 1;

        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            return 1;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            return 1;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;

        if (strcmp(args[0], "export") == 0) {
			if(args[1])
				setenv(args[1],args[2],1);
			return 1;
		}

       // printf("lalalalala\n");

      //  args[1] = NULL;
        /* 外部命令 */
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程 */
            execvp(args[0], args);
            /* execvp失败 */
            return 255;
        }
        /* 父进程 */
        else
        	wait(NULL);
	return 1;
}

int exec_pipe(char *args[128], int total_cnt_pipe, int curr_pipe, int num_args[max_pipe]){

	if(curr_pipe == 0)
		execlp(args[num_args[0] + 1], args[num_args[0] + 1], NULL);

	int filedes[2];
	
	pipe(filedes);
	int pid = fork();
	if (pid == 0){
 		dup2(filedes[1], STDOUT_FILENO);
	 	close(filedes[1]);
	 	close(filedes[0]);
	 	exec_pipe(args, total_cnt_pipe, curr_pipe - 1, num_args);
	 	}
	else{
 		wait(NULL);
 		dup2(filedes[0], STDIN_FILENO);
 		close(filedes[0]);
 		close(filedes[1]);
 		char *temp[128];

 		temp[0] = args[num_args[curr_pipe] + 1];
 		//printf("%s\n",temp[0]);
 		//printf("%s\n", temp[0] + 5);
 		int i;
 		for (i = 0; *temp[i] != '|' && args[num_args[curr_pipe] + 1 + i] != NULL; i++){
           	for (temp[i+1] = temp[i] + 1; temp[i+1] != NULL; temp[i+1]++){
               	if (*temp[i+1] == '\0') {
                	temp[i+1]++;
                   	break;
               	}
            }
        }
        temp[i] = NULL;
        if (curr_pipe == total_cnt_pipe)
        {
        	exec_cmd(temp);
        }
        else{
        	exec_cmd(temp);
        	_exit(0);
        }
	}
	return 1;
}

