#include<unistd.h>
#include<stdio.h>
int main(int arg, char **argv){

    char *wanting_dir="./";
    if(arg > 1){wanting_dir = argv[1];}
    printf("%s\n",wanting_dir);
    char *cmd[3] = {"ls",wanting_dir,NULL};
    int ret=execvp(cmd[0], cmd); 
}