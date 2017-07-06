
#ifndef PLANETDISPLAYLIST_H
#define PLANETDISPLAYLIST_H
#include <gtk/gtk.h>
#include "wrapper.h"


typedef struct planetDisplayList PlanetDisplayList;

struct planetDisplayList
{
    GtkWidget * label;
    GtkWidget * check;
    //TODO: add image for active planets
    int state;                              //0=local 1=active
    int checked;                            //0=unchecked 1=checked
    planet_type *planet;
    PlanetDisplayList * next;
};

//Förslag på vad vi kanske behöver
PlanetDisplayList *createDisplayListNode(planet_type newPlanet);
PlanetDisplayList* addfirstToDisplayList(PlanetDisplayList *head, PlanetDisplayList *newPlanet);
//init
//remove
//empty
//find
//check
//uncheck
//setActive
//setNotActive

#endif /* PLANETDISPLAYLIST_H */
