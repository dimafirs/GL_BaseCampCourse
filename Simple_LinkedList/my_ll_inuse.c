#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_linkedlist.h"

struct my_mega_str {
    char name[20];
    int age;
    struct list_node node;
};
    
int main(int argc, char *argv[]){

    /* Static structure initialization */
    struct my_mega_str john = {.name="John", .age = 20, .node = LIST_NODE_INIT(john.node)}; 
    struct my_mega_str mike = {.name="Mike", .age = 21, .node = LIST_NODE_INIT(mike.node)};
    
    /* Dynamic structure initialization */
    struct my_mega_str *bruce = malloc(sizeof(*bruce));
    strcpy(bruce->name, "Bruce");
    bruce->age = 18;
    INIT_LIST_NODE(&bruce->node);

    struct my_mega_str *kate = malloc(sizeof(*kate));    
    strcpy(kate->name, "Kate");
    kate->age = 19;
    INIT_LIST_NODE(&kate->node);
    
    /* Init the head of our list */
    LIST_NODE(head);
    /* Add nodes to list, order is head-kate-bruce-mike-john */
    list_add_entry(&john.node, &head);
    list_add_entry(&mike.node, &head);
    list_add_entry(&bruce->node, &head);
    list_add_entry(&kate->node, &head);

    /*Initialize iterator and temporary entry*/    
    struct list_node *iter;
    struct my_mega_str *tmp;

    /* Print out list in forward order */
    printf("Forward: \n");
    list_for_each(iter, &head){
        tmp = list_entry(iter, struct my_mega_str, node);
        printf("Member name is %s and he's(she's) %d\n", tmp->name, tmp->age);
    }
    
    /* Delete Bruce entry from list */
    list_del_entry(&bruce->node);
    free(bruce);

    /* Print out list in reverse order without Bruce */
    printf("\nReverse: \n");
    list_for_each_rev(iter, &head){
        tmp = list_entry(iter, struct my_mega_str, node);
        printf("Member name is %s and he's(she's) %d\n", tmp->name, tmp->age);
    }

    exit(0);
}
