/*List.h*/
#ifndef LIST_H
#define LIST_H
#include "wrapper.h"
typedef struct Node Node;

/*List Node struct*/
struct Node
{
    Node * prev;
    planet_type * planet;
    Node * next;
};

/*List head struct*/
typedef struct
{
    Node * head;
    Node * tail;
}ListHead;

/*Creates a listHead and returns it*/
ListHead * createListHead();

/*Creates a node, inserts the planet in it
  and returns the node*/
Node * createNode (planet_type * planet);

/*Adds a Node to the list*/
Node * addNode (ListHead * list, planet_type * planet);

/*Deletes a node from the list*/
int deleteNode (ListHead * list, char * name);

/*Searches for a node*/
Node * searchForNode (Node* node, char * name);

/*Delete list*/
void deleteList(ListHead *list);

#endif /* LIST_H */




// /*List.h*/
// #ifndef LIST_H
// #define LIST_H
// #include "wrapper.h"
// typedef struct Node Node;
//
// /*List Node struct*/
// struct Node
// {
//     Node * prev;
//     planet_type * planet;
//     Node * next;
// };
//
// /*List head struct*/
// typedef struct
// {
//     Node * head;
//     Node * tail;
// }ListHead;
//
// /*Creates a listHead and returns it*/
// ListHead * createListHead();
//
// /*Creates a node, inserts the planet in it
//   and returns the node*/
// Node * createNode (planet_type planet);
//
// /*Adds a Node to the list*/
// Node * addNode (ListHead * list, planet_type  planet);
//
// /*Deletes a node from the list*/
// int deleteNode (ListHead * list, char * name);
//
// /*Searches for a node*/
// Node * searchForNode (Node* node, char * name);
//
// /*Delete list*/
// void deleteList(ListHead *list);
//
// #endif /* LIST_H */
