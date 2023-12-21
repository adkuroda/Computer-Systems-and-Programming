/* 
 * Name: Adilet Kuroda  
 * GNumber:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "r1.h"

/* Iterating the nodes in list
 * Print out the number stored in each node
 * Feel free to pick any printing format
 */
void print_list(List *list){
    Node * current = list -> head;
    while (current != NULL){
        printf("%d ", current->num);
        current = current -> next;
    }
    printf("\n");
}

/* Insert a new node at the front of the linked list for integer num
 * Remeber to update the node count
 * The list might have zero node to start with
 */
void insert_at_front(List *list, int num){
    Node *node;
    if (list != NULL){
        if ((node = malloc(sizeof(Node)))!= NULL){
            node ->num = num;
            node -> next = list ->head;
            list -> head = node;
        }
    }
}

/* =============================================
 * Optional clear_list() function
 *
 * Deallocate the storage pointed by list 
 * =============================================
 */
void clear_list(List *list){
    while(list -> head != NULL){
        Node *current = list -> head;
        list -> head = current ->next;
        free(current);
    }
    free(list);
}


/* Return only the bit at index from integer value
 * The return value should be either 1 or 0
 * Assume the index of LSB (rightmost) is 0
 * Assume the index of MSB (leftmost) is 31
 * Assume the index is always valid (no need to check)
 */
int get_bit(int value, int index){
    int mask = 0x1;
    value = value >> index;
    value &= mask;
    return value;
}

/* Set the bit at index of integer value to be zero
 * Return the changed value
 * Assume the index of LSB (rightmost bit) is 0
 * Assume the index of MSB (leftmost bit) is 31
 * Assume the index is always valid (no need to check)
 */
int clear_bit(int value, int index){
  return -1;
}

/* =============================================
 * Provided main() function
 * =============================================
 */
int main(int argc, char *argv[]) {

  //initialize an empty list of zero nodes
  List *num_list = malloc(sizeof(List));
  num_list->head = NULL;
  num_list->count = 0;

  //build the list
  printf("============Linked list checking==========\n"); 
  for (int i=0; i<10; i++){
    int n = i * 5; 
    //feel free to change the numbers used here, 
    //e.g. int n = rand()%200
    insert_at_front(num_list, n);
  }

  //check the list contents
  print_list(num_list);

  // optional: deallocate the list
  clear_list(num_list);

  //check get_bit()
  printf("============get_bit() checking==========\n"); 

  int x = 0xABCD1234;
  printf("integer: %08X\n", x);
  for (int i=31; i>=0; i--){
    //printf("integer: %08X, index:%d, bit:%d\n", x, i, get_bit(x,i));
    printf("%d", get_bit(x,i));
    if (i%4==0) printf(" ");
  }
  printf("\n");
 
  //check clear_bit()
  printf("============clear_bit() checking==========\n"); 

  for (int i=31; i>=0; i-=4){
    printf("integer: %08X, clear bit index:%d, result:%08X\n", 
	x, i, clear_bit(x,i));
  }

  return 0;
}

