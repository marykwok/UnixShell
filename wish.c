#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char* paths;

void error(){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
}

//generate executable argv
void parsecmd(char* cmd, char* cmdargs, char**argv){
	char* arg0 = cmd;
	argv[0] = arg0;
	int i = 2;
	const char *argn = strsep(&cmdargs, " ");              
	while(argn!=NULL){
		++i;
                argv = realloc(argv, i*sizeof(char*));
                argv[i-2]=(char*)argn;
                argn = strcat(strsep(&cmdargs, " "),"\0");
	}
	argv[i-1] = NULL;
}

void cd(char* cmdargs){
	char *newdir = strsep(&cmdargs, " ");
        if(strsep(&cmdargs," ")!=NULL||newdir==NULL){ 
		error();
		return;
	}
	int rc = chdir(newdir);
	if (rc == 1) error();
}

void redirect(){
	
}

void callcmd(char *cmd, char *cmdargs){
	char *paths_c = malloc(strlen(paths)+1);        //generate path
        strcpy(paths_c, paths);
        char *currpath = malloc(strlen(paths)+1);       //check if necessary
        do{
	       	strcpy(currpath, strsep(&paths_c," ")); 
                char *cmdpath = strcat(strcat(currpath, "/"), cmd);
                //check if char *cmdpath = strcat(strcat(strcpy(strsep(&paths_c," "), "/"), cmd); works
                if(access(cmdpath, X_OK)==0){           //verify path
			char ** argv = malloc(sizeof(char*)*2);
                        char * filepath = malloc(sizeof(char*));
			parsecmd(cmd,cmdargs, argv);
                        pid_t childpid = fork();	//execute command
			if(childpid<0) printf("fork for execv() failed!!!\n");
			else if(childpid==0){
				execv(cmdpath, argv);
                                error();
                        } else if(childpid>0) wait(NULL);
                        free(argv);
               }else error();
	}while(paths_c!=NULL);
        free(paths_c);
        free(currpath);
}

void execmd(char* cmdargs){
        if(cmdargs[strlen(cmdargs)-1]=='\n')
            cmdargs[strlen(cmdargs)-1]='\0';
	char *cmd = strcat(strsep(&cmdargs," "),"\0");
	if(strcmp("exit",cmd)==0){       	               //exit
                if(cmdargs!=NULL){
			error();
			return;
		}
		free(paths);
		exit(0);
        } else if(strcmp("cd", cmd)==0){        	        //cd
		cd(cmdargs);
	} else if(strcmp("path", cmd)==0){      	        //path
		if(cmdargs==NULL){
			strcpy(paths, "");
			return;
		}
		paths = realloc(paths, sizeof(cmdargs)+1);
		strcpy(paths, cmdargs);
	} else if(strcmp("loop", cmd)==0){

	} else{                                          	//executed command
		callcmd(cmd, cmdargs);
	}
}

int main(int argc, char *argv[]){
	paths = malloc(5); strcpy(paths,"/bin");
	size_t s = 0;
	char *cmdargs = NULL;
    	if(argc>2) exit(1);
	if(argc==2){
		int i = 1;
		FILE *f = fopen(argv[1], "r");
		while(getline(&cmdargs,&s,f)>=0){		//TODO:getline() usage
			//printf("executing command ##%s\n", cmdargs);
			execmd(cmdargs);
			++i;
		}
		free(paths);
		exit(0);
	}else{
		while(1){
			printf("wish> ");
			getline(&cmdargs, &s, stdin);
			execmd(cmdargs);
		}
	}
	return 0;
}

