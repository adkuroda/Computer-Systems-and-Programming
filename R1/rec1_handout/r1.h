/* Definitions for Structs for this Recitation */
#ifndef R1_H
#define R1_H


/* Node Struct Definition */
typedef struct node {
  int num;  /* Number stored in node */
  struct node *next;  /* Pointer connecting to the next node in the list */
} Node;

/* List Struct Definition */
typedef struct list_struct {
  Node *head; /* Singly Linked List */
  int count;     /* Number of nodes in list */
} List;

#endif /* R1_H */
