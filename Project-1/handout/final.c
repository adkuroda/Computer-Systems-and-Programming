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
int insert_ascend(Queue * queue, Process *process);
Queue * queue_init();
int set_flags_zero(int flag);
int set_bit_to_one(int flag, int index);
int insert_at_end(Schedule *schedule, Process *process);
Process *select_time_starvation(Schedule * schedule);
int smallest_time_pid(Schedule * schedule);
Process * get_smallest_process(Schedule *s, int pid);

/* Create and initialize a struct Schedule. 
 * Returns a pointer to the new Schedule or NULL on any error.
 */
Schedule *scheduler_init() {
  Schedule * rt = malloc(sizeof(Schedule));
  Queue * q_ready = queue_init();
  Queue * q_stop = queue_init();
  Queue * q_wait = queue_init();
  /*Ensure memery is allocated accordingly*/
  if (q_ready == NULL || q_stop == NULL || q_wait == NULL || rt == NULL){ 
    return NULL;
  }
  /*If memery allocated accordingly, it initializes the values*/
  else{
    rt -> ready_queue = q_ready;
    rt -> stopped_queue = q_stop;
    rt -> waiting_queue = q_wait;
    return rt;
  }
}
/* Add the process into ready_queue ascending order
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_add(Schedule *schedule, Process *process) {
  if ((schedule == NULL) || (process == NULL)){ // ensure valid parameters 
    return -1;
  }
  int i = 0, mask = 1;
  for (i = 1; i <= 3; i++){
    if ((process -> flags) & (mask << i)){
      process -> flags = set_flags_zero(process -> flags); // sets all six flags to zero
      process -> flags = set_bit_to_one(process -> flags, 2); // sets the ready flag to 1 
      return insert_ascend(schedule -> ready_queue, process);
    }
  }
  return -1;
}

/* Receives the process from the CPU and puts it into the Waiting Queue
 * Follow the specification for this function.
 * Returns a 0 on success or -1 on any error.
 */

int scheduler_wait(Schedule *schedule, Process *process, int io_time) {
  if (schedule == NULL || process == NULL){
    return -1;
  }
  process -> wait_remaining = io_time;
  process -> flags = set_flags_zero(process -> flags); // sets all six flags to zero
  process -> flags = set_bit_to_one(process -> flags, 6); // sets the waiting flag to 1 
  return insert_at_end(schedule, process);
}

/* Receives a process from the CPU and moves it to the stopped queue.
 * Follow the specification for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_stop(Schedule *schedule, Process *process) {
  if (schedule == NULL || process == NULL){
    return -1;
  }
  process ->flags = set_flags_zero(process -> flags);
  process -> flags = set_bit_to_one(process ->flags, 4);
  return insert_ascend(schedule ->stopped_queue, process);
}

/* Move the process with matching pid from Stopped to Ready.
 * Follow the specification for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int scheduler_continue(Schedule *schedule, int pid) {
  if (schedule == NULL){
    return -1;
  }
  Process * current = schedule -> stopped_queue -> head;
  if (current == NULL){
    return -1;
  }
  if (current -> pid == pid){
    schedule -> stopped_queue -> head = current -> next;
    current -> next = NULL;
    current -> flags = set_flags_zero(current -> flags); // sets all six flags to zero
    current -> flags = set_bit_to_one(current -> flags, 2); // sets the ready flag to 1 
    schedule -> stopped_queue ->count -= 1;
    return scheduler_add(schedule, current);
  }
  while (current -> next != NULL){
    if (current ->next -> pid == pid){
      current = current -> next -> next;
      current -> next = NULL;
      current -> flags = set_flags_zero(current -> flags); // sets all six flags to zero
      current -> flags = set_bit_to_one(current -> flags, 2); // sets the ready flag to 1 
      schedule -> stopped_queue ->count -= 1;
      return scheduler_add(schedule, current);
    }
    current = current -> next;
  }
  return -1;
}

/* Receives the process from the CPU for removal.
 * Follow the specification for this function.
 * Returns its exit code (from flags) on success or a -1 on any error.
 */
int scheduler_finish(Process *process) {
  if (process == NULL){
    return -1;
  }
  int exit_code = process -> flags;
  if (process -> time_remaining == 0){
    exit_code = exit_code >> FLAG_BITS; // Make sure it work for unsigned int
    
    free(process ->command);
    process -> next = NULL;
    free(process);
  }
  return (exit_code >= 0) ? exit_code : -1;
}

/* Create a new Process with the given information.
 * - Malloc and copy the command string, don't just assign it!
 * Follow the specification for this function.
 * Returns the Process on success or a NULL on any error.
 */
Process *scheduler_generate(char *command, int pid, int time_remaining, int is_sudo) {
  Process *ret =  malloc(sizeof(Process)); // return value 
  if ((ret != NULL) || (command != NULL)){ // ensure memore is allocated properly and valid parameter
    if ((ret -> command = malloc(sizeof(strlen(command)+1)*sizeof(char))) == NULL){
      return NULL;
    }
  }
  if (strncpy(ret -> command, command, sizeof(strlen(command))) == NULL){ // ensure copied accordingly 
    return NULL;
  }
    // initialize all the values and return the Process struct rt
  ret -> pid = pid;
  ret -> time_remaining = time_remaining;
  ret -> time_last_run = clock_get_time();
  ret -> wait_remaining = 0;
  ret -> flags = 0;
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


/* Select the next process to run from Ready Queue.
 * Follow the specification for this function.
 * Returns the process selected or NULL if none available or on any errors.
 */
Process *scheduler_select(Schedule *schedule) {
  if (schedule == NULL){ // Ensure valid parameter 
    return NULL;
  }
  if(schedule -> ready_queue -> head == NULL){
    return NULL;
  }
  Process * current = select_time_starvation(schedule);
  if (current != NULL){
    return current;
  }
  return get_smallest_process(schedule, smallest_time_pid(schedule));
}

Process * get_smallest_process(Schedule *schedule, int pid){
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



/* Updates the first process in the waiting queue by decrementing waiting_time.
 * If that process is done (waiting_time is 0), move it to the ready queue.
 * Follow the specification for this function.
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
  schedule -> waiting_queue -> count -= 1;
  schedule ->waiting_queue -> head = process-> next;

  process -> flags = set_flags_zero(process -> flags);
  process -> flags |= READY;
  return scheduler_add(schedule, process);
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

/* Completely frees all allocated memory in the scheduler
 * Follow the specification for this function.
 */
void scheduler_free(Schedule *scheduler) {
  // if (scheduler == NULL){
  //   return;
  // }
  // if (scheduler -> ready_queue != NULL){
  //   while (scheduler ->ready_queue -> head != NULL){

  //   }
  // }
  return;
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

/* Helper function: Create and initialize struct Queue
 * Returns struct Queue if succeed, else NULL*/
Queue * queue_init(){
  Queue * ret = malloc(sizeof(Queue));
  if (ret != NULL){
    ret -> head = NULL;
    ret -> count = 0;
    return ret;
  }
  return NULL;
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



