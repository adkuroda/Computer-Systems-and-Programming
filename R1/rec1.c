#include <stdio.h>

typedef struct node{
    int num;
    struct node * next;
}Node;

typedef struct list_struct {
    Node * head;
    int count;
} List;
void print_list (List *list){
    Node * current = list -> head;
    if (list != NULL){
        while (current != NULL){
            printf("%d ", current->num);
            current = current -> next;
        }
    }
}
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
int main(){
    List * my_list = malloc(sizeof(List));
    my_list -> head = NULL;
    my_list -> count = 0;
}