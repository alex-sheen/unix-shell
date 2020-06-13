#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

int is_not_empty(char *str)
{
    int res = 0;
    int i = 0;
    while(str[i] != '\n' && str[i] != '\0' && str[i] != EOF)
    {
        if(str[i] >= 33 && str[i] <= 126)
        {
            res = 1;
        }
        i++;
    }
    return res;
}

int num_occurrences(char *str, char ch)
{
    int count = 0;
    int i = 0;
    while(str[i] != '\0' && str[i] != EOF)
    {
        if(str[i] == ch)
        {
            count++;
        }
        i++;
    }
    return count;
}
int get_param_num(char *command, char *delim)
{
    char *tok;
    int count = 0;
    tok = strtok(command, delim);
    while(tok != NULL)
    {
        count++;
        tok = strtok(NULL, delim);
    }
    return count;
}

void set_params(char *command, char *delim, char *arr[])
{
    char *tok;
    int index = 0;
    tok = strtok(command, delim);
    while(tok != NULL)
    {
        arr[index] = tok;
        index++;
        tok = strtok(NULL, delim);
    }
}

void run_command(char *commands[], char *old_buff, int output_fd)
{
    pid_t forkret = 0;
    if((forkret = fork()) == 0)
    {
        if(execvp(commands[0], commands) < 0)
        {
            myPrint("An error has occurred\n");
            exit(1);
        }
  
    }
    else
    {
        int status;
        waitpid(forkret, &status, 0);
        if (WIFEXITED(status)) 
        {  
            if(WEXITSTATUS(status) == 0)
            {
                myPrint(old_buff);
            }   
            else
            {
                exit(1);
            }      
        }
        else
        {
            exit(1);
        }
        close(output_fd);
    }
}

void exec_command(char *commands[], char *path, int plus)
{
    pid_t forkret = 0;
    if((forkret = fork()) == 0)
    {
        int output_fd = 0;
        char *old_buff = NULL;
        if(path != NULL)
        {
            if(plus)
            {
                FILE* input_fd = fopen(path, "r");
                if(input_fd != NULL)
                {
                    int file_size = 0;
   
                    fseek(input_fd, 0, SEEK_END);
                    file_size = ftell(input_fd);
                    fseek(input_fd, 0, SEEK_SET);

                    char buff[file_size+1];
                    char ch;
                    int index = 0;
                    while ((ch = fgetc(input_fd)) != EOF)
                    {
                        buff[index] = ch;
                        index++;
                    }
                    buff[file_size] = '\0';
                    buff[file_size-1] = '\n';
                    old_buff = strdup(buff);
                }
            }
            
            output_fd = open(path, O_RDONLY);
            if(output_fd >= 0 && plus == 0)
            {
                myPrint("An error has occurred\n");
                exit(1);
            }
                close(output_fd);
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                output_fd = creat(path, mode);
                if(output_fd < 0)
                {
                    close(output_fd);
                    myPrint("An error has occurred\n");
                    exit(1);
                }
                dup2(output_fd, STDOUT_FILENO);
        }
        run_command(commands, old_buff, output_fd);
        
        close(output_fd);
        exit(0);
    }

    else
    {
        int status;
        waitpid(forkret, &status, 0);
    }
}

void do_cd(char *command[])
{
    if(strlen(command[0]) > 2)
    {
        myPrint("An error has occurred\n");
    }
    if(command[1] == NULL)
    {
        char *home = getenv("HOME");
        if(chdir(home) != 0)
        {
            myPrint("An error has occurred\n");
        }
    }

    else if(command[2] != NULL)
    {
        myPrint("An error has occurred\n");
    }

    else if(chdir(command[1]) != 0)
    {
        myPrint("An error has occurred\n");
    }
}

void do_pwd(char *command[])
{
    if(command[1] != NULL)
    {
        myPrint("An error has occurred\n");
    }
    else
    {
        char buff[100];
        if(getcwd(buff, 100) == NULL)
        {
            myPrint("An error has occurred\n");
        }
        else
        {
            myPrint(buff);
            myPrint("\n");
        }
    }
}

void parse_command(char *params[], char *path, int plus)
{
        if(params[0] == NULL)
        {
            return;
        }

        else if(strcmp(params[0], "exit") == 0)
        {
            if(params[1] == NULL && path == NULL)
            {
                exit(0);
            }
            else
            {
                myPrint("An error has occurred\n");
            }
        }

        else if(strcmp(params[0], "cd") == 0)
        {
            if(path == NULL)
            {
                do_cd(params);
            }
            else
            {
                myPrint("An error has occurred\n");
            }
        }

        else if(strcmp(params[0], "pwd") == 0)
        {
            if(path == NULL)
            {
                do_pwd(params);
            }
            else
            {
                myPrint("An error has occurred\n");
            }
        }

        else 
        {
            exec_command(params, path, plus);
        }
}

void exec_line(char* line)
{
    char *templine = strdup(line);
    int num_commands = get_param_num(templine, ";");
    char *commands[num_commands];
    set_params(line, ";", commands);

   
    for(int i = 0; i < num_commands; i++)
    {
        char *path = NULL;
        int error = 0;
        int plus = 0;
        if(strstr(commands[i], ">+") != NULL)
        {
            if(num_occurrences(commands[i], '>') > 1)
            {
                myPrint("An error has occurred\n");
                return;
            }
            commands[i] = strtok(commands[i], ">+");
            path = strtok(NULL, ">+");

            if(path == NULL || is_not_empty(path) != 1)
            {
                myPrint("An error has occurred\n");
                error = 1;
                return;
            }

            path = strtok(path, " \t");
            plus = 1;
        }
        else if(strchr(commands[i], '>') != NULL)
        {
            if(num_occurrences(commands[i], '>') > 1)
            {
                myPrint("An error has occurred\n");
                error = 1;
                return;
            }
            commands[i] = strtok(commands[i], ">");
            path = strtok(NULL, ">");

            if(path == NULL || strchr(path, '>') != NULL)
            {   
                myPrint("An error has occurred\n");
                error = 1;
            }
            else if(is_not_empty(path) != 1)
            {
                myPrint("An error has occurred\n"); 
                error = 1;
            }
            path = strtok(path, " \t");
            
        }
        if(error == 0) 
        {
            char *c2 = strdup(commands[i]);
            int num_params = get_param_num(c2, " \t");

            char *params[num_params + 1];
            set_params(commands[i], " \t", params);
            params[num_params] = NULL;

            parse_command(params, path, plus);
        }

    }
}

int main(int argc, char *argv[]) 
{
    char cmd_buff[514];
    char *pinput;

    FILE* fd = stdin;

    if(argc == 2)
    {
        if((fd = fopen(argv[1], "r")) == NULL)
        {
            myPrint("An error has occurred\n");
            exit(1);
        }

        pinput = fgets(cmd_buff, 514, fd);

        while (pinput != NULL)
        {
            if(!pinput) {
                exit(0);
            }

            int bad_line = 0;
            if(strchr(pinput, '\n') == NULL)
            {
                bad_line = 1;
                char ch;
                char *str = "";
                char *old = strdup(pinput);
                
                while ((ch = fgetc(fd)) != '\n' && ch != EOF)
                {
                    char new_str[sizeof(str) + 1];
                    strcpy(new_str,str);

                    char chtostr[2];
                    chtostr[0] = ch;
                    chtostr[1] = '\0';

                    strcat(new_str, chtostr); 
                    str = new_str;
                }
  
                char *new_pinput = malloc(sizeof(old) + sizeof(str));
                strcpy(new_pinput, old);
                strcat(new_pinput,str);

                pinput = new_pinput;
            }
            
            if(bad_line)
            {
                myPrint(pinput);
                myPrint("\n");
                myPrint("An error has occurred\n");
            }

            else if(is_not_empty(pinput))
            {
                pinput = strtok(pinput, "\n");
                myPrint(pinput);
                myPrint("\n");
                exec_line(pinput);
            }
            pinput = fgets(cmd_buff, 514, fd);
        }
    
    fclose(fd);
    exit(0);
    }

    if(argv[1] == NULL)
    {
        while (1) {
            myPrint("myshell> ");
            pinput = fgets(cmd_buff, 514, stdin);
            if (!pinput) {
                exit(0);
            }

            pinput = strtok(pinput, "\n");
            exec_line(pinput);
        }
    }
    else {
        myPrint("An error has occurred\n");
        exit(1);
    }
}
