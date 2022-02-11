#include <stdio.h>
#include <sys/types.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

char* paths;
int cmdcount;
//throw error
void error(){
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
}


//Change Directories
void cd(char* cmdargs){
	char *tokptr, *newdir = strtok_r(cmdargs, " \t", &tokptr);
        if(strtok_r(NULL," \t", &tokptr)!=NULL||newdir==NULL){ 
		error();					//ERROR if args!=1
		return;
	}
	int rc = chdir(newdir);
	if (rc == 1) error();
}

// Call command
void callcmd(char *cmd, char *cmdargs){
	char *pathptr, *filepath = NULL,
	     *paths_c = malloc(strlen(paths)+1);		//generate path
	strcpy(paths_c, paths);
	char *currpath = strtok_r(paths_c, " \t",&pathptr);
	char ** argv = malloc(sizeof(char*)*2);
        //parsecmd(cmd, cmdargs, argv, &filepath);
	char  *argn=NULL, *tokptr_f, *tokptr_c;
        argv[0] = cmd;
        if(cmdargs==NULL){
		argv[1]=NULL;
	}else {
                char *cmdargs_c = strsep(&cmdargs,">");//find redirection
                if(cmdargs!=NULL){
			filepath=strtok_r(cmdargs," >",&tokptr_f);
                        if(filepath==NULL||
                        strtok_r(NULL," ",&tokptr_f)!=NULL){
				filepath = "NOFILE";
                                error();                //ERROR if filepath!=1
                        }
		}
                for(int i=2; ; ++i, cmdargs_c = NULL){//parse arguments
			argn = strtok_r(cmdargs_c, " ",&tokptr_c);
                        argv = realloc(argv, i*sizeof(char*));
                        argv[i-1]=argn;
			if(argn==NULL)
				break;
		}
        }
	if(currpath==NULL){ error(); 				//ERROR
		free(paths_c); return;}
	int accessed = 1;
	while(currpath!=NULL&&accessed==1){
		char *cmdpath=malloc(strlen(currpath)+strlen(cmd)+2);
		strcpy(cmdpath,currpath);
		strcat(cmdpath, "/");
		strcat(cmdpath, cmd);
		accessed = access(cmdpath, X_OK);
                if(accessed==0){           	//verify path
			pid_t childpid = fork();		//execute command
			if(childpid<0) printf("fork for execv() failed!!!\n");
			else if(childpid==0){
				if(filepath!=NULL){                     //redirect
                                	if(strcmp("NOFILE", filepath)==0) exit(0);
					close(STDOUT_FILENO);
                                	open(filepath,O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
				}
				execv(cmdpath, argv);
				error();
				exit(0);			//ERROR if execv fails
                        } else if(childpid>0){
			       	wait(NULL);
			}
               }else error();					//ERROR if invalid pathls
		free(cmdpath);
		currpath = strtok_r(NULL, " \t\n",&pathptr);
	}
        free(paths_c); 
}

//Finds "$loop" and replaces it with loopcount variable
void variableloop(int loopcount, char* cmdargs_c){
	char *loopvar;
	if((loopvar=strstr(cmdargs_c, "$loop"))==NULL) return;
	char *loopcountstr=malloc(sizeof(char*));
	sprintf(loopcountstr,"%i",loopcount);
	*(loopvar)='\0';
	loopvar= loopvar+5;
	strcat(cmdargs_c, loopcountstr);
	strcat(cmdargs_c,loopvar); 
	free(loopcountstr);
}

void execmd(char* allargs){
	if(allargs[strlen(allargs)-1]=='\n') allargs[strlen(allargs)-1]='\0';
	char *cmd, *cmdargs;
	cmd = strtok_r(allargs," \t\n\r",&cmdargs);
	if(cmd==NULL){
		if(cmdcount==0) return;
		error(); return;				//ERROR no commands
	} else if(strcmp("exit",cmd)==0){			//exit
                if(strtok_r(NULL, " \t\n\r", &cmdargs)!=NULL){
			error(); return;
		}
		free(paths);
		exit(0);
        } else if(strcmp("cd", cmd)==0){        	        //cd
		cd(cmdargs);
	} else if(strcmp("path", cmd)==0){      	        //path
		paths = realloc(paths, sizeof(cmdargs)+1);
                strcpy(paths, cmdargs);
		if(strtok_r(NULL, " \t\n", &cmdargs)==NULL){
			strcpy(paths, "");
			return;
		}
	} else if(strcmp("loop", cmd)==0){			//loop
		char *tmp = strtok_r(NULL, " \t\r\n", &cmdargs);
		if(tmp==NULL||(strcmp("",tmp)==0)){ error(); return;}
		for(int i = 0;i<strlen(tmp);++i)
			if(!isdigit(tmp[i])){error(); return;}
		int loopcount = atoi(tmp);
		if(loopcount<=0){error(); return;}
		for(int i = 1;i <= loopcount;++i){
		       	char* cmdargs_c = malloc(strlen(cmdargs)+1);
			strcpy(cmdargs_c, cmdargs);
			variableloop(i, cmdargs_c);
			execmd(cmdargs_c);
			free(cmdargs_c);
		}
	} else{                                          	//executed command
		callcmd(cmd, cmdargs);
	}
}

int main(int argc, char *argv[]){
	paths = malloc(5); strcpy(paths,"/bin");
	cmdcount = 0;
	size_t s = 0;
	char *cmdargs = NULL;
    	if(argc>2){
	       	error(); exit(1);				//ERROR batch>1
	}
	if(argc==2){
		FILE *f = fopen(argv[1], "r");
		if(f==NULL){ error(); exit(1);}			//ERROR file invalid
		while(getline(&cmdargs,&s,f)>=0){
			int i = 0;
			while(isspace(cmdargs[i])){
				cmdargs= &cmdargs[i+1];
				if(i==strlen(cmdargs)) --cmdcount;
			}
			++cmdcount;
			execmd(cmdargs);
		}
		if(cmdcount==0) error();			//ERROR file empty
		free(paths);
		exit(0);
	}else{
		while(1){
			printf("shell> ");
			getline(&cmdargs, &s, stdin);
			execmd(cmdargs);
		}
		free(paths);
	}
	return 0;
}

