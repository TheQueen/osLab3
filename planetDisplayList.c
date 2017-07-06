#include "planetDisplayList.h"

PlanetDisplayList *createDisplayListNode(planet_type newPlanet)
{
  PlanetDisplayList *newNode;
  //TODO: malloc
  newNode->label = NULL;
  newNode->check = NULL;
  //TODO: add for one more variable
  newNode->state = 0;
  newNode->checked = 0;
  //TODO: copy planet
  newNode->next = NULL;
  return newNode;
}

//adds first, returns new head
PlanetDisplayList* addfirstToDisplayList(PlanetDisplayList *head, PlanetDisplayList *newPlanet)
{
  newPlanet->next = head;
  return newPlanet;
}
