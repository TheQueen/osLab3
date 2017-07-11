#include "planetDisplayList.h"

PlanetDisplayList *createDisplayListNode(planet_type *newPlanet)
{
  //TODO: Stina hur initialisera GTK sakerna?
  PlanetDisplayList *newNode = (PlanetDisplayList*)malloc(sizeof(PlanetDisplayList));
  newNode->label = NULL;
  newNode->check = NULL;
  //TODO: add for one more variable?
  newNode->state = 0;
  newNode->checked = 0;
  newNode->planet = newPlanet;
  newNode->next = NULL;
  return newNode;
}

//adds first, returns new head
PlanetDisplayList* addFirstToDisplayList(PlanetDisplayList *head, PlanetDisplayList *newPlanet)
{
  newPlanet->next = head;
  return newPlanet;
}
