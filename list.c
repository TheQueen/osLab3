/*List.c*/
#include "list.h"

/*Creates a listHead and returns it*/
ListHead * createListHead()
{
    ListHead * list;
    if((list = (ListHead *) malloc(sizeof (ListHead))) == NULL)
    {
        printf("head malloc failed\n");
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;

    return list;
}

/*Creates a node, inserts the planet in it
  and returns the node*/
Node * createNode (planet_type * planet)
{
    Node * node;
    if ((node = (Node *) malloc(sizeof(Node))) == NULL)
    {
        printf("malloc Node faile. Planet: %s", planet->name);
        return NULL;
    }

    node->next = NULL;
    node->prev = NULL;
    //node->planet = planet;

    if ((node->planet = malloc(sizeof(planet_type))) == NULL)
    {
        printf("malloc planet failed. Planet: %s", planet->name);
        return NULL;
    }
    int i;
    for ( i = 0; planet->name[i] != '\0'; i++)
    {
        node->planet->name[i] = planet->name[i];
    }
    node->planet->name[i] = '\0';
    node->planet->sx = planet->sx;
    node->planet->sy = planet->sy;
    node->planet->vx = planet->vx;
    node->planet->vy = planet->vy;
    node->planet->mass = planet->mass;
    node->planet->life = planet->life;
    for ( i = 0; planet->pid[i] != '\0'; i++)
    {
        node->planet->pid[i] = planet->pid[i];
    }
    node->planet->pid[i] = '\0';
    node->planet->radius = planet->radius;

    return node;
}

/*Adds a Node to the list*/
Node * addNode (ListHead * list, planet_type * planet)
{
    Node *ptr = createNode(planet);
    if ( list->head == NULL )
    {
        list->head = ptr;
        list->tail = list->head;
    }
    else
    {
        Node * temp = list->tail;
        list->tail = ptr;
        list->tail->prev = temp;
        temp->next = list->tail;
    }
    return ptr;
}

/*Deletes a node from the list*/
int deleteNode (ListHead * list, char * name)
{
    if (list->head == NULL)
    {
        return -1;
    }

    Node * temp = searchForNode(list->head, name);
    if (temp == NULL)
    {
        return -2;
    }
    else
    {
        if(temp->prev != NULL)
        {
            if(temp->next != NULL)
            {
                temp->prev->next = temp->next;
                temp->next->prev = temp->prev;
            }
            else
            {
                list->tail = temp->prev;
                temp->prev->next = NULL;
            }
        }
        else
        {
            if(temp->next != NULL)
            {
                list->head = temp->next;
                temp->next->prev = NULL;
            }
            else
            {
                list->head = NULL;
                list->tail = NULL;
            }
        }
    }
    free(temp->planet);
    free(temp);
    temp->planet = NULL;
    temp = NULL;
    return 0;
}

/*Searches for a node*/
Node * searchForNode (Node* node, char * name)
{
    if (node->next == NULL)
    {
        return NULL;
    }
    if(node->planet->name == name)
    {
        return node;
    }

    return searchForNode(node->next, name);

}

void deleteList(ListHead * list)
{
    if(list->head == NULL)
    {
        return;
    }

    Node * temp = list->tail;

    if(list->head == temp)
    {
        list->head = NULL;
    }
    else
    {
        list->tail = temp->prev;
    }
    free(temp->planet);
    free(temp);
    temp->planet = NULL;
    temp = NULL;
    deleteList(list);
}






// /*List.c*/
// #include "list.h"
//
// /*Creates a listHead and returns it*/
// ListHead * createListHead()
// {
//     ListHead * list;
//     if((list = (ListHead *) malloc(sizeof (ListHead))) == NULL)
//     {
//         printf("head malloc failed\n");
//         return NULL;
//     }
//
//     list->head = NULL;
//     list->tail = NULL;
//
//     return list;
// }
//
// /*Creates a node, inserts the planet in it
//   and returns the node*/
// Node * createNode (planet_type planet)
// {
//     Node * node;
//     planet_type * p;
//     if ((node = (Node *) malloc(sizeof(Node))) == NULL)
//     {
//         printf("malloc Node faile. Planet: %s", planet.name);
//         return NULL;
//     }
//     if ((p = (planet_type*) malloc (sizeof(planet_type))) == NULL)
//     {
//     printf("malloc Node faile. Planet: %s", planet.name);
//     return NULL;
//     }
//
//     node->next = NULL;
//     node->prev = NULL;
//
//     p->life = planet.life;
//     p->mass = planet.mass;
//     int i;
//     for ( i = 0; planet.name[i] != '\0'; i++)
//     {
//         p->name[i] = planet.name[i];
//     }
//     p->name[i] = '\0';
//
//     for ( i = 0; planet.pid[i] != '\0'; i++)
//     {
//         p->pid[i+1] = planet.pid[i];
//     }
//     p->pid[i+1] = '\0';
//     p->radius = planet.radius;
//     p->sx = planet.sx;
//     p->sy = planet.sy;
//     p->vx = planet.vx;
//     p->vy = planet.vy;
//
//     node->planet = p;
//
//     return node;
// }
//
// /*Adds a Node to the list*/
// Node * addNode (ListHead * list, planet_type planet)
// {
//     Node * p = createNode(planet);
//     if ( list->head == NULL )
//     {
//         list->head = p;
//         list->tail = list->head;
//     }
//     else
//     {
//         Node * temp = list->tail;
//         list->tail = p;
//         list->tail->prev = temp;
//         temp->next = list->tail;
//     }
//     return p;
// }
//
// /*Deletes a node from the list*/
// int deleteNode (ListHead * list, char * name)
// {
//     if (list->head == NULL)
//     {
//         return -2;
//     }
//
//     Node * temp = searchForNode(list->head, name);
//     if (temp == NULL)
//     {
//         return -1;
//     }
//     else
//     {
//         if(temp->prev != NULL)
//         {
//             if(temp->next != NULL)
//             {
//                 temp->prev->next = temp->next;
//                 temp->next->prev = temp->prev;
//             }
//             else
//             {
//                 list->tail = temp->prev;
//                 temp->prev->next = NULL;
//             }
//         }
//         else
//         {
//             if(temp->next != NULL)
//             {
//                 list->head = temp->next;
//                 temp->next->prev = NULL;
//             }
//             else
//             {
//                 list->head = NULL;
//                 list->tail = NULL;
//             }
//         }
//     }
//
//     free(temp->planet);
//     free(temp);
//     temp->planet = NULL;
//     temp = NULL;
//     return 0;
// }
//
// /*Searches for a node*/
// Node * searchForNode (Node* node, char * name)
// {
//     if (node->next == NULL)
//     {
//         return NULL;
//     }
//     //TODO: fix this if
//     if(node->planet->name == name)
//     {
//         return node;
//     }
//     else
//     {
//         searchForNode(node->next, name);
//     }
//     return NULL;
// }
//
// void deleteList(ListHead * list)
// {
//     if(list->head == NULL)
//     {
//         return;
//     }
//
//     Node * temp = list->tail;
//
//     if(list->head == temp)
//     {
//         list->head = NULL;
//     }
//     else
//     {
//         list->tail = temp->prev;
//     }
//     free(temp->planet);
//     free(temp);
//     temp->planet = NULL;
//     temp = NULL;
//     deleteList(list);
// }
