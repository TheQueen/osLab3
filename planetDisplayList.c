#include "planetDisplayList.h"

PlanetDisplayList* createDisplayListNode(planet_type *newPlanet)
{
  //TODO: Stina hur initialisera GTK sakerna?
  PlanetDisplayList *newNode = (PlanetDisplayList*)malloc(sizeof(PlanetDisplayList));
  newNode->label = NULL;
  newNode->check = NULL;
  //TODO: add for one more variable?
  newNode->state = 0;
  newNode->checked = 0;

  if ((newNode->planet = malloc(sizeof(planet_type))) == NULL)
  {
      printf("malloc planet failed. Planet: %s", newPlanet->name);
      exit(EXIT_FAILURE);
  }
  int i;
  for ( i = 0; newPlanet->name[i] != '\0'; i++)
  {
      newNode->planet->name[i] = newPlanet->name[i];
  }
  newNode->planet->name[i] = '\0';
  newNode->planet->sx = newPlanet->sx;
  newNode->planet->sy = newPlanet->sy;
  newNode->planet->vx = newPlanet->vx;
  newNode->planet->vy = newPlanet->vy;
  newNode->planet->mass = newPlanet->mass;
  newNode->planet->life = newPlanet->life;
  for ( i = 0; newPlanet->pid[i] != '\0'; i++)
  {
      newNode->planet->pid[i] = newPlanet->pid[i];
  }
  newNode->planet->pid[i] = '\0';
  newNode->planet->radius = newPlanet->radius;

  newNode->next = NULL;
  return newNode;
}

//adds first, returns new head
PlanetDisplayList* addFirstToDisplayList(PlanetDisplayList *head, PlanetDisplayList *newPlanet)
{
  newPlanet->next = head;
  return newPlanet;
}

PlanetDisplayList* findPlanet(PlanetDisplayList *node, char *name)
{
  if(node == NULL)
  {
    return NULL;
  }
  else if(!strcmp(node->planet->name, name))
  {
    return node;
  }
  else
  {
    return findPlanet(node->next, name);
  }
}

PlanetDisplayList *removePlanet(PlanetDisplayList *node, char *name)
{
  if(node == NULL)
  {
    return NULL;
  }
  if(!strcmp(node->planet->name, name))
  {
    //TODO: Stina destroy label
    //TODO: free planet
    PlanetDisplayList *next=NULL;
    next = node->next;
    free(node);
    node = NULL;
    return next;
  }

  node->next = removePlanet(node->next, name);

  return node;
}
