#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int pid = fork();
    if (pid == 0){
        execl("/bin/ls", "ls", NULL);
        printf("children \n");
        exit(0);
    }
    printf("Child PID %d\n", pid);
}

void free() {
  Node * current = head;
  while (current != NULL){
    head  = current -> next;
    current ->cmd_line  = NULL;
    free(current -> cmd_line);
    current = NULL; 
    free(current);
    current = head;
  }
}