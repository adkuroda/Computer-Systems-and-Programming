 /* Name: Adilet Kuroda
 * GNumber: G01253384
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "structs.h"
#include "constants.h"
#include "scheduler.h"

/*Prototype for additional helper functions */
/*Insert struct Process into a given Queue in ascending order*/
int insert_ascend(Queue * queue, Process *process);
/*Sets all the 'State Flags' to zero*/
int set_flags_zero(int flag);
/*Changes the number bit to 1 on given index and returns the number*/
int set_bit_to_one(int flag, int index);
/*Insert a process at the end of ready_queue*/
int insert_at_end(Schedule *schedule, Process *process);
/*Checks if the ready queue has a process that faces time starvation and return it*/
Process *select_time_starvation(Schedule * schedule);
/*Determines the process with smallest time and returns it's PID*/
int smallest_time_pid(Schedule * schedule); 
/*Removes and retuns process with smallest time based on PID*/
Process * get_smallest_process(Schedule *s, int pid);

/* Create and initialize a struct Schedule. Ensures memory is properly allocated accordingly.
 * Returns a pointer to the new Schedule or NULL on any error.
 */
Schedule *scheduler_init() {
  Schedule * rt = malloc(sizeof(Schedule));
  Queue * q_ready = malloc(sizeof(Queue));
  if (q_ready != NULL){
    q_ready -> head = NULL;
    q_ready -> count = 0;
  }
  Queue * q_stop = malloc(sizeof(Queue));
  if (q_stop != NULL){
    q_stop -> head = NULL;
    q_stop -> count = 0;
  }
  Queue * q_wait = malloc(sizeof(Queue));
  if (q_wait != NULL){
    q_wait -> head = NULL;
    q_wait -> count = 0;
  }
  if (rt == NULL){ 
    return NULL;
  }
  else{
    rt -> ready_queue = q_ready;
    rt -> stopped_queue = q_stop;
    rt -> waiting_queue = q_wait;
    return rt;
  }
}
/* Add the process into ready_queue ascending order if the process 
 * in the state of CREATED, READY OR RUNNING
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_add(Schedule *schedule, Process *process) {
  /*Parameter validation*/
  if ((schedule == NULL) || (process == NULL)){ 
    return -1;
  }
  int i = 0, mask = 1;
  for (i = 1; i <= 3; i++){
    /*Checks if the process is CREATED, READY OR RUNNING STATE*/
    if ((process -> flags) & (mask << i)){
      process -> flags = set_flags_zero(process -> flags); // sets all six flags to zero
      process -> flags = set_bit_to_one(process -> flags, 2); // sets the ready flag to 1 
      return insert_ascend(schedule -> ready_queue, process);
    }
  }
  return -1;
}
/* Receives the process from the CPU and puts it into the Waiting Queue 
 * Changes the process state to WAITING and sets the wait_remaining time 
 * based on provided parameter. 
 * Returns a 0 on success or -1 on any error.
 */

int scheduler_wait(Schedule *schedule, Process *process, int io_time) {
  /*Parameter validation*/
  if (schedule == NULL || process == NULL){
    return -1;
  }
  process -> wait_remaining = io_time;
  process -> flags = set_flags_zero(process -> flags); // sets all six flags to zero
  process -> flags = set_bit_to_one(process -> flags, 6); // sets the waiting flag to 1
  /*Utilizes helper function and inserts the updated process at the end of ready_queue*/ 
  return insert_at_end(schedule, process);
}

/* Receives a process from the CPU and moves it to the stopped queue
 * which will be inserted in ascending order based on PID. Changes 
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_stop(Schedule *schedule, Process *process) {
  /*Parameter Validation*/
  if (schedule == NULL || process == NULL){
    return -1;
  }
  /*Set the flag waiting to 1 and rest of the state flags to 0*/
  process ->flags = set_flags_zero(process -> flags);
  process -> flags = set_bit_to_one(process ->flags, 4);
   /*Insert ascending order to ready_queue with helper function insert_ascend(...)*/
  return insert_ascend(schedule ->stopped_queue, process);
}

/* If stopped_queue has a process with provided pid, removes the given 
 * process and inserts into ready queue. 
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_continue(Schedule *schedule, int pid) {
  /*Parameter validation*/
  if (schedule == NULL){
    return -1;
  }
  /*Process current keeps track of iterator*/
  Process * current = schedule -> stopped_queue -> head;
  if (current == NULL){
    return -1;
  }
  /*Checks if the first node contains a process with same pid*/
  if (current -> pid == pid){
    schedule -> stopped_queue -> head = current -> next;
    current -> next = NULL;
    current -> flags = set_flags_zero(current -> flags); // sets all six flags to zero
    current -> flags = set_bit_to_one(current -> flags, 2); // sets the ready flag to 1 
    schedule -> stopped_queue ->count -= 1;
    return scheduler_add(schedule, current);
  }
  /*iterates through stopped_queue and finds if there a process with same pid as parameter*/
  while (current -> next != NULL){
    if (current ->next -> pid == pid){
      current = current -> next -> next;
      current -> next = NULL;
      current -> flags = set_flags_zero(current -> flags); // sets all six flags to zero
      current -> flags = set_bit_to_one(current -> flags, 2); // sets the ready flag to 1 
      schedule -> stopped_queue ->count -= 1;
      /*scheduler_add function insert the process that is removed from stopped_queue to ready queue*/
      return scheduler_add(schedule, current);
    }
    current = current -> next;
  }
  return -1;
}

/* If provided process has 0 time remaining, memory allocated for the given 
 * process will be released and exit code is returned if it is positive integer 
 * Returns its exit code (from flags) on success or a -1 on any error.
 */
int scheduler_finish(Process *process) {
  /*Input validation*/
  if (process == NULL){
    return -1;
  }
  int exit_code = process -> flags; // Extract exit code 
  /*Ensure process has 0 time_remaining*/
  if (process -> time_remaining == 0){
    exit_code = exit_code >> FLAG_BITS; // Make sure it work for unsigned int
    /*Free all the allocated memory*/
    free(process ->command);
    process -> next = NULL;
    free(process);
    return (exit_code >= 0) ? exit_code : -1; // check if the exit code is positive
  }
  return -1;
}

/* Create a new Process with the given information. Ensures memory is properly 
 * allocated for the process. 
 * Returns the Process on success or a NULL on any error.
 */
Process *scheduler_generate(char *command, int pid, int time_remaining, int is_sudo) {
  Process * ret = malloc(sizeof(Process)); // return value 
  if ((ret != NULL) || (command != NULL)){ // ensure memore is allocated properly and valid parameter
    if ((ret -> command = malloc(sizeof(strlen(command)+1)*sizeof(char))) == NULL){ 
      return NULL;
    }
  }
  if (strncpy(ret -> command, command, sizeof(strlen(command))) == NULL){ // ensure copied accordingly 
    return NULL;
  }
  /*Initialize all the values based on provided parameters*/
  ret -> pid = pid;
  ret -> time_remaining = time_remaining;
  ret -> time_last_run = clock_get_time();
  ret -> wait_remaining = 0;
  ret -> flags = 0;
  ret -> next = NULL;
  switch (is_sudo){
    case 1:
      ret -> flags |= SUDO;
      ret -> flags |= CREATED;
      break;
    case 0:
      ret -> flags |= CREATED;
      break;
    }
    return ret;
}
/* Select the next process to run from Ready Queue based on 2 possible criteria: 
 * 1. Select if the process faces time starvation
 * 2. If no starvation, select based on smallest time remaining in the process. 
 * Returns the process selected or NULL if none available or on any errors.
 */
Process *scheduler_select(Schedule *schedule) {
  if (schedule == NULL){ // Ensure valid parameter 
    return NULL;
  }
  if(schedule -> ready_queue -> head == NULL){
    return NULL;
  }
  Process * current = NULL;
  /*Checks if there is any process that faces time starvation and returns that process*/
  current = select_time_starvation(schedule);
  if (current != NULL){
    return current;
  }
  /*Helper function select and returns a process with smallest time remaining*/
  return get_smallest_process(schedule, smallest_time_pid(schedule));
}

/* Updates the first process in the waiting queue by decrementing waiting_time.
 * If that process is done (waiting_time is 0), move it to the ready queue.
 * Returns the process selected or NULL if none available or on any errors.
 */
int scheduler_io_run(Schedule *schedule) {
  if (schedule == NULL){
    return -1;
  }
  if (schedule -> waiting_queue -> head == NULL){
    return -1;
  }
  Process * process = schedule -> waiting_queue -> head;
  process -> wait_remaining -= 1;
  /*Ensures waiting time  is 0*/
  if (process -> wait_remaining == 0){
    schedule -> waiting_queue -> count -= 1; // Descreases the size of waiting queue
    schedule ->waiting_queue -> head = process-> next;
    process -> flags = set_flags_zero(process -> flags);
    process -> flags |= READY;
    /*Adds to ready_queue*/
    return scheduler_add(schedule, process);
  }
  return -1;
}

/* Determines how many element given queue has and returns it. 
 * Returns the count of the given queue, or -1 on any errors.
 */
int scheduler_count(Queue *ll) {
  if (ll == NULL){  // ensures valid parameter
    return -1;
  }
  return ll ->count;
}

/* Completely frees all allocated memory in the scheduler iterating 
 * through each queue within the scheduler. 
 */
void scheduler_free(Schedule *scheduler) {
  Queue *queue = scheduler -> ready_queue;
  Process *current = queue -> head;
  while (current != NULL){
    queue -> head = current -> next;
    current -> next = NULL;
    free(current -> command);
    free(current);
    current = queue ->head;
  }
  free(queue);
  queue = scheduler -> waiting_queue;
  current = queue -> head;
  while (current != NULL){
    queue -> head = current -> next;
    current -> next = NULL;
    free(current -> command);
    free(current);
    current = queue ->head;
  }
  free(queue);
  queue = scheduler -> stopped_queue;
  current = queue -> head;
  while (current != NULL){
    queue -> head = current -> next;
    current -> next = NULL;
    free(current -> command);
    free(current);
    current = queue ->head;
  }
  free(queue);
  free(scheduler);

  return;
}
/* Helper function ath determines a process based on PID and returns it 
 * by removing from ready_queue
 * Returns process if succesful, otherwise NULL*/

Process * get_smallest_process(Schedule *schedule, int pid){
  /*Parameter validation*/
  if (schedule == NULL){
    return NULL;
  }
  Process * current = schedule -> ready_queue -> head;
  if (current == NULL){
    return NULL;
  }
  if (current -> pid == pid){
    schedule -> ready_queue -> head = current -> next;
    schedule -> ready_queue -> count -= 1;
    current -> next = NULL;
    current -> flags = set_flags_zero(current -> flags);
    current -> flags = set_bit_to_one(current -> flags, 3);
    return current;
  }
  /*Iterates through the ready_queue and checks if there any process that 
   * has same PID as parameter. */
  while(current -> next != NULL){
    if (current -> next -> pid == pid){
      schedule -> ready_queue -> count -= 1;
      Process * ret = current -> next;
      current -> next = ret -> next;
      ret -> flags = set_flags_zero(ret -> flags);
      ret -> flags = set_bit_to_one(ret -> flags, 3);
      ret -> next = NULL;
      return ret;
    }
    current = current -> next;
  }
  return NULL;

}


/* Determines a process that is facing time starvation based on time last run and current time. 
 * Returns the process if there is time starvation, otherise NULL*/
Process *select_time_starvation(Schedule * schedule){
  if (schedule == NULL){
    return NULL;
  }
  Process * current = schedule -> ready_queue ->head;
  if (current == NULL){
    return NULL;
  }
  int dif = clock_get_time() - current -> time_last_run;
  if(dif >=TIME_STARVATION){
    schedule -> ready_queue -> head = current -> next;
    schedule -> ready_queue ->  count -=1;
    current -> flags = set_flags_zero(current -> flags);
    current -> flags = set_bit_to_one(current -> flags, 3);
    return current;
  }
  while(current -> next != NULL){
    Process * temp = current -> next;
    dif = clock_get_time() - temp -> time_last_run;
    if (dif >= TIME_STARVATION){
      current -> next = temp -> next;
      schedule -> ready_queue ->  count -=1;
      temp -> flags = set_flags_zero(temp -> flags);
      temp -> flags = set_bit_to_one(temp -> flags, 3);
      return temp;
    }
    current = current ->next;
  }
  return NULL;
}
/* Helper function. Iterates through the ready_queue and identifies the process 
 * with smallest time_remaining
 * Returns PID of a process that is has the smallest time remaining */
int smallest_time_pid(Schedule * schedule){
  Process * current = schedule -> ready_queue -> head;
  int pid = current -> pid;
  int min_time = current -> time_remaining;
  current = current -> next; 
  while(current!= NULL){
    if (current -> time_remaining == min_time){
      if (current -> pid < pid){
        pid = current -> pid; 
      }
    }
    if (current -> time_remaining < min_time){
      pid = current -> pid;
      min_time = current -> time_remaining;
    }
    
    current = current -> next;
  }
  return pid;
}

int insert_ascend(Queue *queue, Process *process){
  if ((queue == NULL) || (process == NULL)){ // ensure valid parameters 
    return -1;
  }
  /*In case the list is empty*/
  if (queue-> head == NULL){
    queue -> head = process;
    queue -> count +=1;
    return 0;
  }
  Process * current = queue -> head;
  /*Make sure the first element's pid is not greater than given parameter 
  * since we need the reference to previous node to insert properly*/
  if (current -> pid > process -> pid){
    process -> next = current;
    queue -> head = process;
    queue -> count +=1;
    return 0;
  }    
      /*Insert the within the given list*/
  while (current -> next != NULL){
    if (current -> next -> pid > process -> pid){
      process -> next = current -> next;
      current -> next = process;
      queue -> count +=1;
      return 0;
    } 
    current = current -> next;
  }
      /*Insert at the end of the list*/
  current -> next = process;
  process -> next = NULL;
  queue-> count +=1;
  return 0;
}


/* Helper Function: Sets first 7 least significant bits excluding the 
 * first bit to 0*/
int set_flags_zero(int flag){
  flag &= ~0x7E;
  return flag;
}

/*Helper function: sets sets the bit on given index to 1*/
int set_bit_to_one(int flag, int index){
  flag |= (1 << index);
  return flag;
}

int insert_at_end(Schedule *schedule, Process *process){
  if(schedule == NULL || process == NULL){
    return -1;
  }
  Process *current = schedule -> waiting_queue -> head;
  process -> next = NULL;
  if (current == NULL){
    schedule -> waiting_queue -> head = process;
    schedule -> waiting_queue -> count += 1;
    return 0;
  }
  /*Iterate to the end of the list*/ 
  while(current ->next != NULL){
    current = current -> next;
  }
  /*Insert the item at the end of the list*/
  current -> next = process;
  schedule -> waiting_queue -> count += 1;
  return 0;
}



