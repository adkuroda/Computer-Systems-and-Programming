/* This is the only file you should update and submit. */
/***********************************************************
 * Name: Adilet Kuroda
 * GNumber: G01253384
 ***********************************************************/

#include "shell.h"
#include "parse.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>



/******************** CONSTANTS  ****************************/
#define DEBUG 1

/********************  GLOBALS  *****************************/

 //static const char *shell_path[] = { "./", "/usr/bin/", NULL };
 static const char *built_ins[] = { "quit", "help", "kill", 
                                    "fg", "bg", "jobs", NULL};                  
/* Node for the linked list */
typedef struct background_job_list{
  char *cmd;
  int state;  // 1 running, 0 stopped 
  int is_bg;  // 1 True, 0 False 
  int JID;
  int PID;
  struct background_job_list * next;
} Node;

Node * head = NULL; /*Head pointer for linked list*/
Node * fg_job = NULL; /*Pointer to forground job*/
int num_jobs = 0; /*How background jobs currently active*/
int job_id = 1;   /*JOB ID*/
char status_stopped [] = "Stopped"; /*Status of job*/
char status_running [] = "Running"; /*Status of job*/
int exitcode = 0;

/******************** PROTOTYPES ****************************/
/***** Builtin function prototypes *****/
void static b_quit();
void static b_kill(char *argv[]);
void static b_jobs();
void b_bg(char *cmd, char *argv[]);
void b_fg(char *cmd, char *argv[]);


/*** MASH prototypes ****/
void handler(int sig);
void fg_waitpid(int pid, int status);
void execute_child(char *cmd, char *argv[]);
void file_redirection(Cmd_aux aux, char *cmd, char *argv[]);
void replace(char *argv[]);

/**** Linked List prototypes *****/
Node * create_node(char *cmd, int state, int is_bg, int JID, int PID);
void insert_node(Node * node);
void remove_node(int pid);
void free_list();
Node * get_node_JID(int jobid);
Node * get_node_PID(int jobid);


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
      //  debug_print_parse(cmd, argv, &aux, "main (after parse)");
      /* After parsing: your code to continue from here */
      /*================================================*/
  /*Appendix D: Project handout*/
  struct sigaction action;
  //struct sigaction old;
  memset(&action, 0, sizeof(action));
 // memset(&old,0, sizeof(old));
  action.sa_handler = handler;
  sigaction(SIGCHLD, &action, NULL);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTSTP, &action, NULL);

  /*Note: Signal blockers are similar to code in course book P. 779*/
  sigset_t mask_all, prev_all; 
  sigfillset(&mask_all);
  replace(argv);

  /*built in function quit*/
  if (strcmp(argv[0], built_ins[0]) == 0){
    b_quit();
  }
  /*built in function help*/
  else if (strcmp(argv[0], built_ins[1]) == 0){
    log_help();
  }
   /*built in function kill*/
  else if (strcmp(argv[0], built_ins[2]) == 0){
    b_kill(argv);
  }
   /*built in function fg*/
  else if (strcmp(argv[0], built_ins[3]) == 0){
    b_fg(cmd, argv);
  }
   /*built in function bg*/
  else if (strcmp(argv[0], built_ins[4]) == 0){
    b_bg(cmd, argv);
  }
   /*built in function jobs*/
  else if (strcmp(argv[0], built_ins[5]) == 0){
    b_jobs();
  }
  /*Background and forground Jobs*/
  else{
    int pid = 0;
    if ((pid = fork()) == 0){
      /*In case file redirection, sets proper output or input*/
      if ((aux.in_file != NULL) | (aux.out_file != NULL)){
        file_redirection(aux, cmd, argv);
      }
      setpgid(0,0); // Piazza @836
      execute_child(cmd, argv);
    }
    /*If background job, inserts creates a node and inserts into list*/
    if (aux.is_bg){
      sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      Node * temp = NULL;
      temp  = create_node(cmd, 1, 0, job_id, pid);
      insert_node(temp);
      log_start(pid, LOG_BG, cmd);
      sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
    /*If forground job, create a node that represents foreground job*/
    else if(aux.is_bg == 0){
      sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      fg_job = create_node(cmd, 1, 0, 0, pid);
      log_start(pid, LOG_FG, cmd);
      sigprocmask(SIG_SETMASK, &prev_all, NULL);
      int status = 0;
      int pid_fg = -1;
      pid_fg = waitpid(pid, &status, 0);
      if(pid_fg > 0){
        fg_waitpid(pid, status);
      }
      fg_job = NULL;
    }
    free_options(&cmd, argv, &aux);
    }
    }
  }
  return 0;
}
/******************* SIGNAL HANDLERS  *******************************/
/*This function ensures the forground process handled accordinlgy which 
 * wait until the process completes. Utilizing different macros 
 * different actions are taken based on status.
 */
void fg_waitpid(int pid, int status){
  if (WIFSTOPPED(status)){
    log_ctrl_z();
    if (fg_job != NULL){
      log_job_state(fg_job -> PID, LOG_FG, fg_job -> cmd, LOG_STOP);
      kill(fg_job -> PID, SIGTSTP);
      Node * temp = create_node(fg_job -> cmd, 0, 1, job_id, fg_job -> PID);
      insert_node(temp);
      fg_job = NULL;
    }
  }
  else if (WIFCONTINUED(status)){
    if(fg_job != NULL){
      if (pid == fg_job -> PID){
        fg_job -> state = 0;
          log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_CONT);
      }
    }
  }
  else if (WIFEXITED(status) | WIFSIGNALED(status)){
    if(fg_job != NULL){ 
      if (pid == fg_job -> PID){
        if((WIFEXITED(status))){
          log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_TERM);
          exitcode = WEXITSTATUS(status);
          
        }
        if (WIFSIGNALED(status)){
          log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_TERM_SIG);
        }
        fg_job = NULL;
      }
    }
  }
}
/* I am referencing the code from course book P. 779. I have used similar aproach 
 * provided in the book aformentioned page to block and unblock the signals. 
 * This function handles three types of signals (SIGINT,SIGTSTP, SIGCHLD). 
 * SIGINT: If there is foreground job, it terminates it by sending signal to 
 *         forground process. If there is no fg process, it just does nothing. 
 * SIGTSTP: If there is fg process, it pauses it by sending signal to fg process
 *         and appends the process to end of background  list as stopped process. 
 * SIGCHLD: waits for the process without blocking it if there are no processes is 
 *          reporting its status. Depending on status of signal from waitpid, Utilizing 
 *          different macros that decodes the status, different actions are taken. 
 * Sorry code become too long. I should have broken it down. 
 */

void handler(int sig){
  sigset_t mask_all, prev_all; 
  sigfillset(&mask_all);
  if (sig == SIGINT){ 
    log_ctrl_c();
    if (fg_job != NULL){ 
      kill(fg_job -> PID, SIGINT); 
    }
  }
  /*Handles SIGTSTP sends  */
  else if (sig == SIGTSTP){ 
    log_ctrl_z();
    if (fg_job != NULL){
      log_job_state(fg_job -> PID, LOG_FG, fg_job -> cmd, LOG_STOP);
      kill(fg_job -> PID, SIGTSTP);
      Node * temp = create_node(fg_job -> cmd, 0, 1, job_id, fg_job -> PID);
      insert_node(temp);
      fg_job = NULL;
    }
  }
  else if (sig == SIGCHLD){
    int status = 0;
    int pid = 0;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
      sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
      if (WIFSTOPPED(status)){
        if(fg_job != NULL){
          if (pid == fg_job -> PID){
            fg_job -> state = 0;
            log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_STOP);
          }
        }
        else{
          Node *temp = get_node_PID(pid);
          if (temp != NULL){
            temp ->state = 0;
            log_job_state(pid, LOG_BG, temp -> cmd, LOG_STOP);
          }
        } 
      }
      else if (WIFCONTINUED(status)){
        if (fg_job != NULL){ 
          if (pid == fg_job -> PID){
            fg_job -> state = 0;
            log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_CONT);
          }
        }
        else{
          Node *temp = get_node_PID(pid);
          if (temp != NULL){
            temp ->state = 1;
            log_job_state(pid, LOG_BG, temp -> cmd, LOG_CONT);
          }
        } 
      } 
      else if (WIFEXITED(status) | WIFSIGNALED(status)){
        if(fg_job != NULL){ 
          if (pid == fg_job -> PID){
            fg_job -> state = 0;
            if(WIFEXITED(status)){
              exitcode = WEXITSTATUS(status);
              log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_TERM);
              exitcode = status;
             
            }
            if (WIFSIGNALED(status)){
              log_job_state(pid, LOG_FG, fg_job -> cmd, LOG_TERM_SIG);
            }
            fg_job = NULL;
          }
        }
        else{
          Node * temp = get_node_PID(pid);
          if (temp != NULL){
            if(WIFEXITED(status)){
              log_job_state(pid, LOG_BG, temp -> cmd, LOG_TERM);
            }
            if (WIFSIGNALED(status)){
              log_job_state(pid, LOG_BG, temp -> cmd, LOG_TERM_SIG);
            }
          }

          remove_node(pid);
        } 
      }
      sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
  }
}

/******************** MASH FUNCTIONS ********************************/
/*Iterates through argv and check if there is any patter of $?. If so it replaces 
 * with exit code and logs apropriately.*/

void replace(char *argv[]){
  int i = 0;
  char replacement[]  = "$?";
  while(argv[i] != NULL){
    if(strcmp(argv[i],replacement) == 0){
      char temp[MAXLINE] = {0};  // Note: Similar to logging.c file since I looked there. 
      sprintf(temp, "%d", exitcode); // // Note: Similar to logging.c file since I looked there. 
      char * pt = malloc(strlen(temp)+ 1);
      if (pt != NULL){
        strcpy(pt, temp);
        argv[i] = pt;
      }
      log_replace(i, argv[i]);
      return;
    }
    i++;  
  }
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
  log_command_error(cmd); /*Only time comes here when both execv does not work*/
  exit(1);
}


/*This function handles all file redirection FIX THIS AGAIN. */
void file_redirection(Cmd_aux aux, char *cmd, char *argv[]){
  if (aux.in_file != NULL){
    int fd_in = open(aux.in_file, O_RDONLY, 0600);
    if (fd_in < 0){
      log_file_open_error(aux.in_file);
      exit(0);
    } 
    dup2(fd_in, STDIN_FILENO);
  }
  if (aux.out_file != NULL){
    if(aux.is_append){
      int fd_out = open(aux.out_file, O_RDWR | O_CREAT | O_APPEND, 600);
      if (fd_out < 0){
        log_file_open_error(aux.out_file);
        exit(0);
      }
      dup2(fd_out, STDOUT_FILENO);
    }
    else{
      int fd_out = open(aux.out_file, O_RDWR | O_CREAT | O_TRUNC, 600);
      if (fd_out < 0){
        log_file_open_error(aux.out_file);
        exit(0);
      }
      dup2(fd_out, STDOUT_FILENO);
    }
  }
}



/******************* BUILT IN FUNCTIONS *****************************/
/*Exits the MASH. Frees all the allocated memory. */
/*Buitin function performs quit operation*/ 
void static b_quit(){
  log_quit();
  free_list();
  exit(0);
}
/*Buitin function that sends different signals to jobs based on process ID*/ 
void static b_kill(char *argv[]){
  int pid = atoi(argv[2]);
  int signal = atoi(argv[1]);
  log_kill(signal, pid);
  kill(pid, signal);
}
/*Buitin function displays all the background jobs that is running or
 * stopped */
void static b_jobs(){
  log_job_number(num_jobs);
  Node * current = head;
  while (current != NULL){
    if (current -> state){ 
      log_job_details(current -> JID, current -> PID,
                     status_running, current -> cmd);
    }
    else{
      log_job_details(current -> JID, current -> PID, 
                      status_stopped, current -> cmd);
    }
  current = current -> next;
  }
}

/* This function processes the fg built in function which moves bg job
 * foreground based on jobID. if there is an no such jobID if will log 
 * accordingly. 
 */
void b_fg(char *cmd, char *argv[]){
  int input_jobID = atoi(argv[1]);
  Node * temp = get_node_JID(input_jobID);
  if (temp == NULL){
    log_jobid_error(input_jobID);
  }
  else{
    int pid = temp -> PID;
    if (temp -> state == 0){
        kill(pid, SIGCONT);   
    }
    fg_job = create_node(cmd, 1, 0, 0, pid);
    remove_node(pid);
    log_job_move(pid, LOG_FG, cmd);
    log_job_state(pid, LOG_FG, cmd, LOG_CONT);
    int pid_fg = -1;
    int status = 0;
    pid_fg = waitpid(pid, &status, 0);
    if(pid_fg > 0){
        fg_waitpid(pid, status);
    }
  }
}
/* This function processes the bg built in function which sends a signal 
 * to a background job that is stopped and log accordingly. 
 *  if there is an no such jobID if will log accordingly. 
 */
void b_bg(char *cmd, char *argv[]){
  int input_jobID = atoi(argv[1]);
  Node * temp = get_node_JID(input_jobID);
  if (temp == NULL){
    log_jobid_error(input_jobID);
  }
  else{
    int pid = temp -> PID;
    if (temp -> state == 0){
        kill(pid, SIGCONT); 
        temp ->state = 1;  
        log_job_state(temp -> PID, LOG_BG, temp ->cmd, LOG_CONT);
    }
    log_job_move(pid, LOG_BG, cmd);
  }
}

/******************* LINKED LIST OPERATIONS *************************/
/*Create and initializes a node. Return created node if succeeds. 
 * Otherwise returns NULL*/


Node * create_node(char *cmd, int state, int is_bg, int JID, int PID){
  Node * ret;
  ret  = malloc(sizeof(Node));
  if (ret != NULL){
    ret -> cmd = malloc(sizeof(strlen(cmd))+ 1);
    if (ret -> cmd != NULL){
      strcpy(ret -> cmd, cmd);
      ret -> state = state;
      ret -> is_bg = is_bg;
      ret -> JID = JID;
      ret -> PID = PID;
      return ret;
    }
    return NULL;
  }
  return NULL;
}


/* Creates a node using create_node function ands insert the node in ascending order based on 
 * job ID. This keeps track of how many node added to the list. 
 */
void insert_node(Node * node){
  if (head == NULL){ /*In case it is empty*/
    head = node;
    num_jobs++;
    job_id++;
  }
  else{
    Node * current = head; /*pointer to iterate the list*/
    while(current -> next != NULL){
      current = current -> next;
    }
    current -> next = node;
    node -> next = NULL;
    num_jobs++;
    job_id++;
  }
}


/* Removes the node based on provided job ID or process ID. This function 
 * decrements the number of nodes in the list if it succeed to ensure there is 
 * proper count of jobs in the given list. Frees the memory of the node that is 
 * been deleted.
 */
void remove_node(int pid){
  Node * current = head;
  Node * temp = NULL;
  if (current == NULL){
    return;
  }                      
  if ((current -> PID) == pid){
    head = current -> next;
    current -> cmd = NULL;
    free(current -> cmd);
    free (current);
    num_jobs --;
    return;
  }
  while(current -> next != NULL){
    if ((current -> next -> PID) == pid){
      temp = current -> next;
      current -> next = current -> next -> next;
      temp -> cmd = NULL;
      free(temp -> cmd);
      temp = NULL;
      free(temp);
      num_jobs--;
      return;
    }
    current = current -> next;
  }
}
/*Frees all the allocated memory before it quits the MASH*/
void free_list() {
  Node * current = head;
  while (current != NULL){
    head  = current -> next;
    current ->cmd  = NULL;
    free(current -> cmd);
    current = NULL; 
    free(current);
    current = head;
  }
}

/*This finds and returns pointer to background job based on provided job ID*/
Node * get_node_JID(int jobid){
  Node * current = head;
  while (current != NULL){
    if (jobid == (current -> JID)){
      return current;
    }
    current = current -> next;
  }
  return NULL;
}

/*This find and returns a pointer based on provided process ID*/
Node * get_node_PID(int pid){
  Node * current = head;
  while (current != NULL){
    if (pid == (current -> PID)){
      return current; 
    }
    current = current -> next;
  }
  return NULL;
}


/**************************************************************/

/* T
