/* This is the only file you should update and submit. */

/* Fill in your Name and GNumber in the following two comment fields
 * Name: Adilet Kuroda
 * GNumber:
 */

#include "shell.h"
#include "parse.h"

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>

/*Glabals*/
static int num_jobs = 0; /*Keeps track of number of jobs*/

/*This will be used as linked list to keep track of background job*/
typedef struct background_job_list{
  char *cmd_line;
  int job_ID;
  int process_ID;
  char * state;
  struct background_job_list * next;
}Node;
/*Global pointer to the head of the list*/
Node * head = NULL;
Node * fg_job = NULL;
char status_stopped [] = "Stopped";
char status_running [] = "Running";
int exit_code = 0;
int job_id = 1;


/*Prototypes*/
void execute_child(char *cmd, char *argv[]);
Node * create_node( char *cmd_line, int job_ID, int process_ID, char *state);
int insert_node(char *cmd_line, int job_ID, int process_ID, char* state);
int remove_node(int PID);
void handler(int sig);
int change_status(int pid, int st);
Node * get_node(int pid);


/* Constants */
#define DEBUG 1
/*
 * static const char *shell_path[] = { "./", "/usr/bin/", NULL };
 * static const char *built_ins[] = { "quit", "help", "kill", 
 * "fg", "bg", "jobs", NULL};
*/

/* The entry of your shell program */
int main() {
  char cmdline[MAXLINE];        /* Command line */
  char *cmd = NULL;

  /* Intial Prompt and Welcome */
  log_prompt();
  log_help();


  /* Shell looping here to accept user command and execute */
  while (1) {
    char *argv[MAXARGS];        /* Argument list */
    Cmd_aux aux;                /* Auxilliary cmd info: check parse.h */

    /* Print prompt */
    log_prompt();

    /* Read a line */
    // note: fgets will keep the ending '\n'
    if (fgets(cmdline, MAXLINE, stdin) == NULL) {
      if (errno == EINTR)
        continue;
      exit(-1);
    }

    if (feof(stdin)) {  /* ctrl-d will exit shell */
      exit(0);
    }

    /* Parse command line */
    if (strlen(cmdline)==1)   /* empty cmd line will be ignored */
      continue;     

    cmdline[strlen(cmdline) - 1] = '\0';        /* remove trailing '\n' */

    cmd = malloc(strlen(cmdline) + 1);
    snprintf(cmd, strlen(cmdline) + 1, "%s", cmdline);

    /* Bail if command is only whitespace */
    if(!is_whitespace(cmd)) {
      initialize_argv(argv);    /* initialize arg lists and aux */
      initialize_aux(&aux);
      parse(cmd, argv, &aux); /* call provided parse() */
      
      // if (DEBUG)  /* display parse result, redefine DEBUG to turn it off */
      //   debug_print_parse(cmd, argv, &aux, "main (after parse)");
    
      /* After parsing: your code to continue from here */
      /*================================================*/
      struct sigaction action;
      memset(&action, 0, sizeof(action));
      action.sa_handler = handler;
      sigaction(SIGCHLD, &action, NULL);
      sigaction(SIGINT, &action, NULL);
      sigaction(SIGTSTP, &action, NULL);
      if (strcmp(argv[0],"help") == 0){
        log_help();
      }
      else if (strcmp(argv[0],"quit") == 0){
        log_quit();
        exit(0);
      }
      else if (strcmp(argv[0],"jobs") == 0){
        log_job_number(num_jobs);
        Node * current = head;
        while (current != NULL){
          log_job_details(current -> job_ID, current -> process_ID,
                          current -> state, current -> cmd_line);
          current = current -> next;
        }
        
      }
      else if (strcmp(argv[0],"kill") == 0){
        int temp_pid = atoi(argv[2]);
        int temp_sig = atoi(argv[1]);
        log_kill(temp_sig, temp_pid);
        kill(temp_pid, temp_sig);
        printf("Make sure to test this again");
      }
      else if (strcmp(argv[0],"fg") == 0){
        printf("FG implementation");
      }
      else if (strcmp(argv[0],"bg") == 0){
        printf("bg implementation");
      }
      else if((strcmp(argv[0],"fg/bg/jobs/kill") == 0)){
        printf("implement fg/bg/jobs/kill");
      }
      else{
        if (aux.is_bg){
          int pid_bg = 0;
          if ((pid_bg =fork()) == 0){ 
              execute_child(cmd, argv);
          }
            log_start(pid_bg, LOG_BG, cmd);
            // Handle siganals and so on. 
        } 
        else{
          sigset_t mask_all, prev_all; 
          sigfillset(&mask_all);
          int status_fg = 0;
          int pid_fg = 0;
          if ((pid_fg =fork()) == 0){  
            execute_child(cmd, argv);
          }
          log_start(pid_fg, LOG_FG, cmd);
          waitpid(pid_fg, &status_fg, 0);
          sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
          if (WIFEXITED(status_fg)){
            log_job_state(pid_fg, LOG_FG, cmd, LOG_TERM);
          }
          else if (WIFSIGNALED(status_fg)){
            log_job_state(pid_fg, LOG_FG, cmd, LOG_TERM_SIG);
          }
          sigprocmask(SIG_SETMASK, &prev_all, NULL); /*Resumes signal receiving*/
          }
      }
     free_options(&cmd, argv, &aux);
    }
  }
  return 0;
}

/* This function appends the command line to two different paths and 
 * tries to execute both options. If Neither succeeds, it will exit the 
 * the process with exit code of 1. 
 */
void execute_child(char *cmd, char *argv[]){
  char full_path[] = "/usr/bin/";
  char partial_path[] = "./";
  char * path = strcat(partial_path, argv[0]);
  execv(path, argv); 
  path = strcat(full_path, argv[0]);
  execv(path, argv);
  log_command_error(cmd);
  exit(1);
}

/* Creates a node that represents a list one of the jobs in the process. 
 * Returns struct type pointer node if succeed. Otherwise NULL. 
 */
Node * create_node( char *cmd_line, int job_ID, int process_ID, char *state){
  Node * ret = malloc(sizeof(Node));
  if (ret != NULL){
    ret -> cmd_line = malloc(sizeof(strlen(cmd_line)+1));
    strncpy(ret -> cmd_line, cmd_line, sizeof(strlen(cmd_line)));
    ret -> job_ID = job_ID;
    ret -> process_ID = process_ID;
    ret -> state = malloc(sizeof(strlen(state)+1));
    strncpy(ret -> state, state, sizeof(strlen(cmd_line)));
    return ret;
    
  }
  else{
    return NULL;
  }
}
/* Creates a node using create_node function ands insert the node in ascending order based on 
 * job ID. This keeps track of how many node added to the list. 
 * Returns 1 upon success, otherwise -1. 
 */
int insert_node(char *cmd_line, int job_ID, int process_ID, char* state){
  Node * temp = create_node(cmd_line,job_ID, process_ID, state);
  /*if it is empty list, just adds the new node*/
  if (head == NULL){
    head = temp;
    num_jobs++;
    return 1;
  }
  Node * current = head; /*pointer to iterate the list*/
  if (current -> job_ID > temp -> job_ID){
    temp -> next = current;
    head = temp;
    num_jobs++;
    job_id++;
    return 1;
  }
  else{ 
    /*Steps through the list and ensures inserts ascending order*/
    while (current -> next != NULL){
      if (current ->next -> job_ID > temp -> job_ID){
        temp -> next = current -> next;
        current -> next = temp;
        num_jobs++;
        return 1;
      }
      current = current -> next; // steps to next in list 
    }
    /*if new node has biggest job ID then it will append at the end*/
    current -> next = temp;
    temp -> next = NULL;
  }
  return -1;

}

/* Removes the node based on provided job ID or process ID. This function 
 * decrements the number of nodes in the list if it succeed to ensure there is 
 * proper count of jobs in the given list. 
 * Returns 1 upon success otherwise -1 
 */
int remove_node(int PID){
  Node * current = head;
  if (current == NULL){
    return -1;
  }
  /*Checks if the first item has same PID*/
  if ((current -> process_ID) == PID){
    head -> next = current -> next;
    num_jobs --;
    return 1;
  }
  while(current -> next != NULL){
    if ((current -> next -> process_ID) == PID){
      current -> next = current -> next -> next;
      num_jobs --;
      return 1;
    }
  }
  return -1;
}

/* I am referencing the code from course book P. 779. I have used similar aproach 
 * provided in the book aformentioned page to block and unblock the signals. 
 */

void handler(int sig){
  int status = 0;
  int pid = 0;
  sigset_t mask_all, prev_all; 
  sigfillset(&mask_all);
  /*Handles SIGINT(Ctrl-c) signal. Sends signal to fg job to terminate */
  if (sig == SIGINT){ /*Ignores if there is no forground job*/
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    log_ctrl_c();
    if (fg_job != NULL){ /*Ensures forground job is there*/
      kill(fg_job ->process_ID, SIGINT); /*Sending signal to terminate*/
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
  }
  /*Handles SIGTSTP sends  */
  else if (sig == SIGTSTP){ 
    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
    log_ctrl_z();
    if (fg_job != NULL){  /*Ignores if there is no forground job*/
      kill(fg_job ->process_ID, SIGTSTP);
      insert_node(fg_job -> cmd_line, job_id , fg_job ->process_ID, status_stopped);
      fg_job = NULL;
    }
    sigprocmask(SIG_SETMASK, &prev_all, NULL);
  }
  /*Handles SIGCHILD*/ 
  else if (sig == SIGCHLD){
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
      sigprocmask(SIG_BLOCK, &mask_all, &prev_all); /*Blocks any signal*/
      Node * temp = get_node(pid);
      //if (temp != NULL){
        /*Changes the status of the job if it is stopped*/
        if (WIFSTOPPED(status)){
          log_job_state(pid, LOG_BG, temp -> cmd_line, LOG_STOP);
          change_status(pid, 0);
        }
        /*Chnage the status of the job it continued*/
        else if (WIFCONTINUED(status)){
          log_job_state(pid, LOG_BG, temp -> cmd_line, LOG_CONT);
          change_status(pid, 1);
        }
        /*Removes from background job list in case child is exited normally or terminated by a signal.*/
        else if (WIFEXITED(status) | WIFSIGNALED(status)){
          if (WIFEXITED(status)){
            log_job_state(pid, LOG_BG, temp ->cmd_line, LOG_TERM);
            exit_code = WEXITSTATUS(status);
          }
          else{
            log_job_state(pid, LOG_BG, temp -> cmd_line, LOG_TERM_SIG);
          }
          remove_node(pid);
        }
      //}
      sigprocmask(SIG_SETMASK, &prev_all, NULL); /*Resumes signal receiving*/
    }
  }
}
/* Changes the status of job. If st is 0 then job is stopped by signal and if 
 * st is 1 then job is continued by signal.
 * Returns 1 upon success and -1 otherwise.  
 */
int change_status(int pid, int st){
  Node * current = head;
  while(current != NULL){
    if (current -> process_ID == pid){
      if (st == 0){
        current ->state = status_stopped;
      }
      if (st == 1){
        current -> state = status_running;
      }
      return 1;         
    }
  current = current -> next;
  }  
  return -1;
}
/* Finds the node that corresponds to the pid by iterating through the linked list
 * of background jobs. Return NULL in case, if there is no job with pid, otherwise  
 * returns the node containing the PID. 
 */
Node * get_node(int pid){
  Node * current = head;
  while (current != NULL){
    if(current -> process_ID == pid){
      return current;
    }
    current = current -> next;
  }
  return current;
}
  

