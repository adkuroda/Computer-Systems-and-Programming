/* Do Not Modify This File */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Bit Flags Constants */
/* Note: Feel free to use these if you like. You don't have to. */
#define SUDO            0x1
#define CREATED         0x2
#define READY           0x4
#define RUNNING         0x8
#define STOPPED         0x10
#define TERMINATED      0x20
#define WAITING         0x40

#define EXITCODE_BITS   25
#define FLAG_BITS       7

/* Simulation Constants */
#define MAX_COMMAND     255   /* Max Process Command Length */ 
#define MAX_LINE_LEN    512   /* Max Trace File Line Length */

#define TIME_STARVATION 6     /* The number of time units before it starts starving */

#endif /* CONSTANTS_H */
