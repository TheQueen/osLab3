#include "wrapper.h"

#define MAXSIZE sizeof(planet_type)

//returns: 1 or 0
int threadCreate (void * functionCall, pthread_t threadParam, void * args)
{
    int err =0;
    err = pthread_create(&threadParam, NULL, functionCall, args);
    if(err != 0)
    {
        printf ("cant create thread\n");
    }
    else
    {
        printf ("Successfully created thread!!!\n");
    }
    return err;
}
//returns: 1 or 0
int MQcreate (mqd_t * mq, char * name)
{
    struct mq_attr attributes;
    attributes.mq_flags = 0;
    attributes.mq_maxmsg = 10;
    attributes.mq_msgsize = MAXSIZE;
    attributes.mq_curmsgs = 0;

    *mq=mq_open(name, O_CREAT | O_RDWR, 0666, &attributes);
    if((mqd_t)-1 == *mq)
    {
        printf("Error MQcreate: %s\n", strerror(errno));
        return 0;
    }
    else
    {
        printf("MQcreate succsess\n");
        fflush(stdout);
        return 1;
    }
}
//returns: 1 or 0
int MQconnect (mqd_t * mq, char * name)
{
    *mq = mq_open(name, O_RDWR);
    if((mqd_t)-1 == *mq)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
//returns: bytes read or -1
int MQread (mqd_t * mq, void * refBuffer)
{
    return mq_receive(*mq, refBuffer, 101, NULL);
}
//returns: 0 if ok and -1 if not ok
int MQwrite (mqd_t *mq, void *sendBuffer)
{
    return mq_send(*mq, sendBuffer, MAXSIZE, 0);
}
//returns: 1 or 0
int MQclose(mqd_t * mq, char * name)
{
    return (mq_close(*mq) != -1 && mq_unlink(name) != -1);
}

planet_type  createPlanet(char * name, double xpos, double ypos, double xV, double yV, double mass, int life, char * pid, int r)
{
    planet_type  p;
    //p.name = name;
    int i;
    for ( i = 0; name[i] != '\0'; i++)
    {
        p.name[i] = name[i];
    }
    p.name[i] = '\0';
    p.sx = xpos;
    p.sy = ypos;
    p.vx = xV;
    p.vy = yV;
    p.mass = mass;
    p.life = life;

    for ( i = 0; pid[i] != '\0'; i++)
    {
        p.pid[i] = pid[i];
    }
    p.pid[i] = '\0';
    p.radius = r;
    return p;

}
