/* Do Not Modify this File */

/* Definitions for Structs for this Project */
#ifndef STRUCTS_H
#define STRUCTS_H


/* Process Struct Definition */
typedef struct process_struct {
  char *command;      /* Process Command */
  int pid;            /* Process ID (unique) */
  int flags;          /* Process Flags */
  int time_remaining; /* Time Units Left to Execute */
  int time_last_run;  /* The Last Time the Process was Selected (or time created) */
  int wait_remaining; /* I/O Time in the Waiting Queue Remaining */
  struct process_struct *next; 
} Process;

/* Linked List (Queue) Struct Definition */
typedef struct queue_struct {
  Process *head; /* Singly Linked List */
  int count;     /* Number of items in list */
} Queue;

/* Schedule Struct Definition */
typedef struct schedule_struct {
  Queue *ready_queue;   /* Ready Processes */
  Queue *stopped_queue; /* Stopped Processes */
  Queue *waiting_queue; /* Waiting Processes */
} Schedule;

#endif /* STRUCTS_H */
