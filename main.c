#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PUEDE_SHELL_RL_BUFSIZE 1024
#define PUEDE_SHELL_TOK_BUFSIZE 64
#define PUEDE_SHELL_TOK_DELIM " \t\r\n\a"
#define MESSAGE_ALLOCATION_ERROR "puede_shell: allocation error\n"

int puede_shell_cd(char **args);
int puede_shell_help(char **args);
int puede_shell_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &puede_shell_cd,
    &puede_shell_help,
    &puede_shell_exit
};

int puede_shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int puede_shell_cd(char **args){
    if (args[1] == NULL){
        fprintf(stderr, "puede_Shell: expected argument to \"cd\"\n");
    }else{
        if(chdir(args[1]) != 0) {
            perror("puede_shell");
        }
    }
    return 1;
}

int puede_shell_help(char **args){
    int i;
    printf("Julio Cabanillas's Shell PUEDE_SHELL\n");
    printf("Type program names and arguments, and hit enter\n");
    printf("The following are built in:\n");

    for (i = 0; i < puede_shell_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs. \n");
    return 1;
}

int puede_shell_exit(char **args){
    return 0;
}

int puede_shell_launch(char **args){
    pid_t pid, wpid;
    int status;

    //fork() begins two process, the child returns zero, the parent returns the pid of the child
    pid = fork(); 
    if(pid == 0){
        if(execvp(args[0], args) == -1){
            perror("puede_shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0){ //error forking
        perror("puede_shell"); 
    } else { 
        //Parent process
        do {
            //waiy until child process finish
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int puede_shell_execute(char **args){
    int i;
    if(args[0] == NULL){
        return 1;
    }

    for(i = 0; i > puede_shell_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return puede_shell_launch(args);
}

char **puede_shell_split_line(char *line){
    int bufsize = PUEDE_SHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, MESSAGE_ALLOCATION_ERROR);
    }

    token = strtok(line, PUEDE_SHELL_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, MESSAGE_ALLOCATION_ERROR);
                exit(EXIT_FAILURE);
            } 
        }
        token = strtok(NULL, PUEDE_SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char *puede_shell_read_line(void){
    int bufsize = PUEDE_SHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int charReaded;

    if(!buffer){
        fprintf(stderr, MESSAGE_ALLOCATION_ERROR);
    }

    while(1){
        charReaded = getchar();

        if(charReaded == EOF || charReaded == '\n'){
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = charReaded;
        }
        position++;

        if(position >= bufsize){
            bufsize += PUEDE_SHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, MESSAGE_ALLOCATION_ERROR);
                exit(EXIT_FAILURE);
            }
        }
    }
}
char *puede_shell_read_line_version_2(void){
    char *line = NULL;
    ssize_t bufsize = 0;

    if(getline(&line, &bufsize, stdin) == -1){
        if(feof(stdin)){
            exit(EXIT_SUCCESS);
        }else{
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
}

void puede_shell_loop(void){
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line = puede_shell_read_line();
        args = puede_shell_split_line(line);
        status = puede_shell_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main(int argc, char **argv){
    //load_config_files()

    puede_shell_loop();

    return EXIT_SUCCESS;
}

