#ifndef WRAPPER_H
#define WRAPPER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <pthread.h>

//extern int threadCreate (void * functionCall, int threadParam);
extern int threadCreate (void * functionCall, pthread_t threadParam, void * args);


extern int MQcreate (mqd_t * mq, char * name);
extern int MQconnect (mqd_t * mq, char * name);
extern int MQread (mqd_t * mq, void * refBuffer);
extern int MQwrite (mqd_t * mq, void * data);
int MQclose(mqd_t * mq, char * name);


typedef struct pt {
	char	name[20];               // Name of planet
	double	sx;			// X-axis position
	double	sy;			// Y-axis position
	double	vx;			// X-axis velocity
	double	vy;			// Y-axis velocity
	double	mass;                   // Planet mass
	int	life;                   // Planet life
	char	pid[30];                //ID of creating process
        int     radius;                 //Radius of the planet
} planet_type;

planet_type createPlanet(char * name, double xpos, double ypos, double xV, double yV, double mass, int life, char * pid, int r);

#endif /* WRAPPER_H */
