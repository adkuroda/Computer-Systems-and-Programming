/* Do Not Modify This File */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "structs.h"

/* Prototypes in the Scheduler API */
Schedule *scheduler_init();
int scheduler_add(Schedule *schedule, Process *process);
int scheduler_stop(Schedule *schedule, Process *proc);
int scheduler_continue(Schedule *schedule, int pid);
int scheduler_wait(Schedule *schedule, Process *proc, int io_time);
int scheduler_io_run(Schedule *schedule);
Process *scheduler_generate(char *command, int pid, 
                            int cur_priority, int is_sudo);
int scheduler_finish(Process *process);
Process *scheduler_select(Schedule *schedule);
int scheduler_count(Queue *ll);
void scheduler_free(Schedule *schedule);

#endif /* SCHEDULER_H */
