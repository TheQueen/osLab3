#include <cairo.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "list.h"
#include "wrapper.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

static void do_drawing(cairo_t *);
GtkWidget *window;
GtkWidget *darea;
double x = 0.0;
double y = 0.0;
double x2 = 5.0;

#define PLANETIPC "/PlanetLabd"
mqd_t mq;
ListHead * head;
int id = 1;
pthread_mutex_t addOrRemove;

int calc(planet_type * planet)
{
    //truct timespec t = {.tv_sec = 0, .tv_nsec = 900000};
    Node *ptr = head->head;
    double G = 6.67259 * pow(10, -11);
    double dt = 10.0;
    double aToTx = 0.0;
    double aToTy = 0.0;

    if (planet->life == 0)
    {
        return 2.0;
    }
    if (ptr == NULL)
    {
        return 0.0;
    }

    do
    {
        printf("%s / %s \n", ptr->planet->name, planet->name);
        if (!strncmp(ptr->planet->name, planet->name, strlen(planet->name)))
        {

        }
        else
        {
            double x = ptr->planet->sx - planet->sx;
            printf("x: %f \n", x);
            double y = ptr->planet->sy - planet->sy;
            printf("y: %f \n", y);
            double r = sqrt((x * x) + (y * y));
            printf("r: %f \n", r);
            double a1 = G * (ptr->planet->mass / (r * r));
            printf("a1: %f \n", a1);
            double ax = a1 * (x / r);
            printf("ax: %f \n", ax);
            double ay = a1 * (y / r);
            printf("ay: %f \n", x);

            aToTx = aToTx + ax;
            aToTy = aToTy + ay;
        }
        ptr = ptr->next;
    } while (ptr != NULL);


    double newVX = planet->vx + (aToTx * dt);
    printf("newVX: %f \n", newVX);
    double newVY = planet->vy + (aToTy * dt);
    printf("newVY: %f \n", newVY);
    double newSX = planet->sx + (newVX * dt);
    printf("newSX: %f \n", newSX);
    double newSY = planet->sy + (newVY * dt);
    printf("newSY: %f \n", newSY);

    planet->sx = newSX;
    planet->sy = newSY;
    planet->vx = newVX;
    planet->vy = newVY;
    planet->life = planet->life - 1;

    aToTx = 0.0;
    aToTy = 0.0;

    if (planet->life <= 0)
    {
        return 0;
    }
    if (((planet->sx > 800) || (planet->sx < 0)) || ((planet->sy > 600) || (planet->sy < 0)))
    {
        planet->life = -1;
        return -1;
    }
    return 1;
}

/*
void * threadTestFunc(void * planet) {
    int dead = 1;
    while (dead)
    {
        //dead = calc(planet, head->head);
        usleep(1000);
    }

    while (1) {
        if (pthread_mutex_trylock(&myStruct->mutex)) {
            //pthread_mutex_lock(mutex);
            MQwrite(&mq, planet);
            pthread_mutex_unlock(&myStruct->mutex);
            pthread_cond_signal(&myStruct->deadCond);

            deleteNode(head, ((planet_type *) planet)->name);
            return "planet died";
        } else {
            pthread_cond_wait(&myStruct->aliveCond, &myStruct->mutex);
        }
    }
}
*/
void calc2(planet_type * planet)
{
    //truct timespec t = {.tv_sec = 0, .tv_nsec = 900000};
    Node *ptr = head->head;
    double G = 6.67259 * pow(10, -11);
    double dt = 10.0;
    double aToTx = 0.0;
    double aToTy = 0.0;

    if (planet->life == 0) {
        return;
    }
    if (ptr == NULL) {
        return;
    }

    do {
        //printf("%s / %s \n", ptr->planet->name, planet->name);
        if (!strncmp(ptr->planet->name, planet->name, strlen(planet->name)))
        {

        }
        else
        {
            double x = ptr->planet->sx - planet->sx;
            //printf("x: %f \n", x);
            double y = ptr->planet->sy - planet->sy;
            //printf("y: %f \n", y);
            double r = sqrt((x * x) + (y * y));
            //printf("r: %f \n", r);
            double a1 = G * (ptr->planet->mass / (r * r));
            //printf("a1: %f \n", a1);
            double ax = a1 * (x / r);
            //printf("ax: %f \n", ax);
            double ay = a1 * (y / r);
            //printf("ay: %f \n", x);

            aToTx = aToTx + ax;
            aToTy = aToTy + ay;
        }
        ptr = ptr->next;
    } while (ptr != NULL);


    double newVX = planet->vx + (aToTx * dt);
    //printf("newVX: %f \n", newVX);
    double newVY = planet->vy + (aToTy * dt);
    //printf("newVY: %f \n", newVY);
    double newSX = planet->sx + (newVX * dt);
    //printf("newSX: %f \n", newSX);
    double newSY = planet->sy + (newVY * dt);
    //printf("newSY: %f \n", newSY);

    planet->sx = newSX;
    planet->sy = newSY;
    planet->vx = newVX;
    planet->vy = newVY;
    planet->life = planet->life - 1;


    aToTx = 0.0;
    aToTy = 0.0;

    //sleep(1);

    if (planet->life <= 0) {
        return;
    }
    if (((planet->sx > 800) || (planet->sx < 0)) || ((planet->sy > 600) || (planet->sy < 0))) {
        planet->life = -1;
        return;
    }
    return;
}

void * handlePlanet(void * arg)
{
    planet_type * planet = (planet_type *) arg;

    while(planet->life > 0)
    {
        calc2(planet);
        usleep(1000);
    }
    printf("%s\n", planet->pid);

    mqd_t tempmq;
    if (MQconnect(&tempmq, planet->pid) == 0)
    {
        printf("Failed to connect to client!\n");
    }
    if(MQwrite(&tempmq, planet) != 0)
    {
        printf("write: %s\n", strerror(mq));
        printf("could not write to client when dead\n");
    }

    while(1)
    {
        if(pthread_mutex_trylock(&addOrRemove))
        {
            int back = deleteNode(head, planet->name);
            printf("back: %d\n", back);
            pthread_mutex_unlock(&addOrRemove);
            break;
        }
    }

    pthread_exit(NULL);
}
/*
int mapVars()
{
    int fd;
    fd = shm_open(FNAME, O_CREAT | O_RDWR | O_TRUNC, mode);
    if(fd < 0)
    {
        printf("could not open shm_open");
        return 0;
    }

    if (ftruncate(fd, sizeof(MyStruct)) == -1)
    {
        printf("Error on ftruncate to sizeof pthread_cond_t\n");
        return 0;
    }

    myStruct = (MyStruct*) mmap(NULL, sizeof(MyStruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (myStruct == MAP_FAILED )
    {
        printf("Error on mmap\n");
        return 0;
    }

    return 1;
}
*/
/*
int mapVars2()
{
    int fd;
    fd = shm_open(FNAME, O_CREAT | O_RDWR | O_TRUNC, mode);
    if(fd < 0)
    {
        printf("could not open shm_open");
        return 0;
    }

    if (ftruncate(fd, sizeof(MyStruct)) == -1)
    {
        printf("Error on ftruncate to sizeof pthread_cond_t\n");
        return 0;
    }

    myStruct2 = (MyStruct*) mmap(NULL, sizeof(MyStruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (myStruct2 == MAP_FAILED )
    {
        printf("Error on mmap\n");
        return 0;
    }

    return 1;
}
*/
/*
void * mqAndPlanet(void * arg) {
    int loop = 1;
    planet_type * planet; //byt ut mot structen
    if (!MQcreate(&mq, PLANETIPC)) {
        printf("Failed to create server!\n");
        return NULL;
    }
    while (loop) {

        pthread_mutex_lock(&myStruct->mutex);
        MQread(&mq, planet);
        pthread_mutex_unlock(&myStruct->mutex);

        addNode(head, planet);
        threadCreate(threadTestFunc, id, planet);
        id++;
        pthread_cond_signal(&myStruct->aliveCond);
        pthread_cond_wait(&myStruct->aliveCond, &myStruct->mutex);
    }
    return NULL;
}
*/
void * getPlanets(void * arg)
{
    planet_type tempP;
    int bytes_read;
    Node * ptr;
    if (!MQcreate(&mq, PLANETIPC))
    {
        printf("Failed to create server!\n");
        return NULL;
    }
    printf("created server!\n");

    while(1)
    {
        bytes_read = MQread(&mq, &tempP);
        if(bytes_read != -1)
        {
            pthread_mutex_lock(&addOrRemove);
            ptr = addNode(head, &tempP);
            pthread_mutex_unlock(&addOrRemove);

            printf("Planet %s: proc %s\n", tempP.name, tempP.pid);
            fflush(stdout);
            //TODO: remember that this might not work the way I want it to
            threadCreate(handlePlanet, (pthread_t)tempP.name, ptr->planet);
        }
    }
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    do_drawing(cr); //Launch the actual draw method
    return FALSE; //Return something
}

static void do_drawing(cairo_t *cr) //Do the drawing against the cairo surface area cr
{
    Node * ptr = head->head;
    cairo_set_source_rgb(cr, 0, 0, 0); //Set RGB source of cairo, 0,0,0 = black
    while (ptr != NULL) {
        planet_type * p = ptr->planet;
        cairo_arc(cr, p->sx, p->sy, p->radius, 0, 2 * 3.1415);
        cairo_fill(cr);
        //threadTestFunc (p);
        ptr = ptr->next;
    }

}

void on_frame_tick(GtkWidget * widget, GdkFrameClock * frame_clock, gpointer user_data) //Tick handler to update the frame
{
    gdk_frame_clock_begin_updating(frame_clock); //Update the frame clock
    gtk_widget_queue_draw(darea); //Queue a draw event
    gdk_frame_clock_end_updating (frame_clock); //Stop updating fram clock
}

int main(int argc, char *argv[]) //Main function
{
    head = createListHead();
    //if(!mapVars()){return (EXIT_SUCCESS);}
    //printf("%d", myStruct->test);
    //fflush(stdout);
    //myStruct->test = 2;
    //printf("%d", myStruct->test);
    //fflush(stdout);
    /*
    planet_type p1 = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, 1, 5);
    planet_type p2 = createPlanet("p2", 200.0, 300.0, 0.0, 0.0008, 1000.0, 100000000, 2, 5);
    planet_type p3 = createPlanet("p3", 400.0, 300.0, 0.0, -0.0008, 1000.0, 1000000, 3, 5);
    planet_type p4 = createPlanet("p4", 300.0, 200.0, -0.0008, 0.0, 1000.0, 1000000, 4, 5);
    planet_type p5 = createPlanet("p3", 300.0, 400.0, 0.0008, 0.0, 1000.0, 1000000, 5, 5);

    head = createListHead();
    addNode(head, &p1);
    addNode(head, &p2);
    addNode(head, &p3);
    addNode(head, &p4);
    addNode(head, &p5);
    */

    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me
    gtk_init(&argc, &argv); //Initialize GTK environment
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //Create a new window which will serve as your top layer
    darea = gtk_drawing_area_new(); //Create draw area, which will be used under top layer window
    gtk_container_add(GTK_CONTAINER(window), darea); //add draw area to top layer window
    g_signal_connect(G_OBJECT(darea), "draw",
            G_CALLBACK(on_draw_event), NULL); //Connect callback function for the draw event of darea
    g_signal_connect(window, "destroy", //Destroy event, not implemented yet, altough not needed
            G_CALLBACK(gtk_main_quit), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); //Set position of window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); //Set size of window
    gtk_window_set_title(GTK_WINDOW(window), "GTK window"); //Title
    gtk_widget_show_all(window); //Show window
    gtk_widget_add_tick_callback(darea, (GtkTickCallback)on_frame_tick, NULL, (GDestroyNotify)1); //Add timer callback functionality for darea
    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me

    /*
    Node * ptr = head->head;
    int id = 1;
    while (ptr != NULL)
    {
        threadCreate(threadTestFunc, id, ptr);
        ptr = ptr->next;
        id++;
    }
    */

    threadCreate(getPlanets, 0, NULL);
   // pthread_exit(NULL);
    //planet_type p1 = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, 1, 5);
    //handlePlanet(&p1);

    gtk_main(); //Call gtk_main which handles basic GUI functionality
    //pthread_exit(NULL);
    return 0;
}

/*#include <cairo.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "list.h"
#include "wrapper.h"
#include <time.h>
#include <unistd.h>
//#include <pthread.h>
//#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

static void do_drawing(cairo_t *);
GtkWidget *window;
GtkWidget *darea;
double x = 0.0;
double y = 0.0;
double x2 = 5.0;

#define PLANETIPC "/PlanetLa"
#define FNAME "/mapVars"
mqd_t mq;
pthread_condattr_t attrcond;
pthread_mutexattr_t attrmutex;
int mode = S_IRWXU | S_IRWXG | S_IRWXO;

ListHead * head;
int id = 1;

typedef struct mutexConds
{
    pthread_mutex_t mutex;
    pthread_cond_t aliveCond;
    pthread_cond_t deadCond;
    int test;
} MyStruct;

MyStruct * myStruct;
MyStruct * myStruct2;

int calc(Node * planet, Node * ptr) {
    //truct timespec t = {.tv_sec = 0, .tv_nsec = 900000};

    double G = 6.67259 * pow(10, -11);
    double dt = 10.0;
    double aToTx = 0.0;
    double aToTy = 0.0;


    if (planet->planet->life == 0) {
        return 2.0;
    }
    if (ptr == NULL) {
        return 0.0;
    }

    do {
        printf("%d / %d \n", ptr->planet->pid, planet->planet->pid);
        if (ptr->planet->pid == planet->planet->pid) {

        } else {
            double x = ptr->planet->sx - planet->planet->sx;
            printf("x: %f \n", x);
            double y = ptr->planet->sy - planet->planet->sy;
            printf("y: %f \n", y);
            double r = sqrt((x * x) + (y * y));
            printf("r: %f \n", r);
            double a1 = G * (ptr->planet->mass / (r * r));
            printf("a1: %f \n", a1);
            double ax = a1 * (x / r);
            printf("ax: %f \n", ax);
            double ay = a1 * (y / r);
            printf("ay: %f \n", x);

            aToTx = aToTx + ax;
            aToTy = aToTy + ay;
        }
        ptr = ptr->next;
    } while (ptr != NULL);


    double newVX = planet->planet->vx + (aToTx * dt);
    printf("newVX: %f \n", newVX);
    double newVY = planet->planet->vy + (aToTy * dt);
    printf("newVY: %f \n", newVY);
    double newSX = planet->planet->sx + (newVX * dt);
    printf("newSX: %f \n", newSX);
    double newSY = planet->planet->sy + (newVY * dt);
    printf("newSY: %f \n", newSY);

    planet->planet->sx = newSX;
    planet->planet->sy = newSY;
    planet->planet->vx = newVX;
    planet->planet->vy = newVY;
    planet->planet->life = planet->planet->life - 1;


    aToTx = 0.0;
    aToTy = 0.0;

    //sleep(1);

    if (planet->planet->life <= 0) {
        return 0;
    }
    if (((planet->planet->sx > 800) || (planet->planet->sx < 0)) || ((planet->planet->sy > 600) || (planet->planet->sy < 0))) {
        planet->planet->life = -1;
        return -1;
    }
    return 1;
}

void * threadTestFunc(void * planet) {
    int dead = 1;
    while (dead)
    {
        dead = calc(planet, head->head);
        usleep(1000);
    }

    while (1) {
        if (pthread_mutex_trylock(&myStruct->mutex)) {
            //pthread_mutex_lock(mutex);
            MQwrite(&mq, planet);
            pthread_mutex_unlock(&myStruct->mutex);
            pthread_cond_signal(&myStruct->deadCond);

            deleteNode(head, ((planet_type *) planet)->name);
            return "planet died";
        } else {
            pthread_cond_wait(&myStruct->aliveCond, &myStruct->mutex);
        }
    }
}

void calc2(planet_type * planet)
{
    //truct timespec t = {.tv_sec = 0, .tv_nsec = 900000};
    Node *ptr = head->head;
    double G = 6.67259 * pow(10, -11);
    double dt = 10.0;
    double aToTx = 0.0;
    double aToTy = 0.0;


    if (planet->life == 0) {
        return 2.0;
    }
    if (ptr == NULL) {
        return 0.0;
    }

    do {
        printf("%d / %d \n", ptr->planet->pid, planet->pid);
        if (ptr->planet->pid == planet->pid) {

        } else {
            double x = ptr->planet->sx - planet->sx;
            printf("x: %f \n", x);
            double y = ptr->planet->sy - planet->sy;
            printf("y: %f \n", y);
            double r = sqrt((x * x) + (y * y));
            printf("r: %f \n", r);
            double a1 = G * (ptr->planet->mass / (r * r));
            printf("a1: %f \n", a1);
            double ax = a1 * (x / r);
            printf("ax: %f \n", ax);
            double ay = a1 * (y / r);
            printf("ay: %f \n", x);

            aToTx = aToTx + ax;
            aToTy = aToTy + ay;
        }
        ptr = ptr->next;
    } while (ptr != NULL);


    double newVX = planet->vx + (aToTx * dt);
    printf("newVX: %f \n", newVX);
    double newVY = planet->vy + (aToTy * dt);
    printf("newVY: %f \n", newVY);
    double newSX = planet->sx + (newVX * dt);
    printf("newSX: %f \n", newSX);
    double newSY = planet->sy + (newVY * dt);
    printf("newSY: %f \n", newSY);

    planet->sx = newSX;
    planet->sy = newSY;
    planet->vx = newVX;
    planet->vy = newVY;
    planet->life = planet->life - 1;


    aToTx = 0.0;
    aToTy = 0.0;

    //sleep(1);

    if (planet->life <= 0) {
        return 0;
    }
    if (((planet->sx > 800) || (planet->sx < 0)) || ((planet->sy > 600) || (planet->sy < 0)))
    {
        planet->life = -1;
        return -1;
    }
    return 1;
}

void * handlePlanet(void * arg)
{
    planet_type * planet = (planet_type*)arg;
    addNode(head, planet);
    while(planet->life > 0)
    {
        calc2(planet);
    }
    //skicka tillbaka planeten
    pthread_exit(NULL);
}

int mapVars()
{
    int fd;
    fd = shm_open(FNAME, O_CREAT | O_RDWR | O_TRUNC, mode);
    if(fd < 0)
    {
        printf("could not open shm_open");
        return 0;
    }

    if (ftruncate(fd, sizeof(MyStruct)) == -1)
    {
        printf("Error on ftruncate to sizeof pthread_cond_t\n");
        return 0;
    }

    myStruct = (MyStruct*) mmap(NULL, sizeof(MyStruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (myStruct == MAP_FAILED )
    {
        printf("Error on mmap\n");
        return 0;
    }

    return 1;
}

void initVars()
{
    pthread_condattr_init(&attrcond);
    pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&myStruct->aliveCond, &attrcond);
    pthread_cond_init(&myStruct->deadCond, &attrcond);

    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&myStruct->mutex, &attrmutex);
}

void * mqAndPlanet(void * arg) {
    int loop = 1;
    planet_type * planet; //byt ut mot structen
    if (!MQcreate(&mq, PLANETIPC)) {
        printf("Failed to create server!\n");
        return NULL;
    }
    while (loop)
    {
        pthread_mutex_lock(&myStruct->mutex);
        MQread(&mq, planet);
        pthread_mutex_unlock(&myStruct->mutex);

        addNode(head, planet);
        threadCreate(threadTestFunc, id, planet);
        id++;
        pthread_cond_signal(&myStruct->aliveCond);
        pthread_cond_wait(&myStruct->aliveCond, &myStruct->mutex);
    }
    return NULL;
}

void * getPlanets(void * arg)
{
    planet_type tempP;
    int bytes_read;

    if (!MQcreate(&mq, PLANETIPC))
    {
        printf("Failed to create server!\n");
        return NULL;
    }

    printf("created server!\n");
    while(1)
    {
        bytes_read = MQread(&mq, &tempP);
        if(bytes_read != -1)
        {
            printf("Planet %d: %s\n", tempP.pid, tempP.name);
            threadCreate(handlePlanet, id, &tempP);
        }
    }
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    do_drawing(cr); //Launch the actual draw method
    return FALSE; //Return something
}

static void do_drawing(cairo_t *cr) //Do the drawing against the cairo surface area cr
{
    Node * ptr = head->head;
    cairo_set_source_rgb(cr, 0, 0, 0); //Set RGB source of cairo, 0,0,0 = black
    while (ptr != NULL) {
        planet_type * p = ptr->planet;
        cairo_arc(cr, p->sx, p->sy, p->radius, 0, 2 * 3.1415);
        cairo_fill(cr);
        //threadTestFunc (p);
        ptr = ptr->next;
    }

}

GtkTickCallback on_frame_tick(GtkWidget * widget, GdkFrameClock * frame_clock, gpointer user_data) //Tick handler to update the frame
{
    gdk_frame_clock_begin_updating(frame_clock); //Update the frame clock
    gtk_widget_queue_draw(darea); //Queue a draw event
    gdk_frame_clock_end_updating (frame_clock); //Stop updating fram clock
}

int main(int argc, char *argv[]) //Main function
{

    head = createListHead();


    planet_type p1 = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, 1, 5);
    planet_type p2 = createPlanet("p2", 200.0, 300.0, 0.0, 0.0008, 1000.0, 100000000, 2, 5);
    planet_type p3 = createPlanet("p3", 400.0, 300.0, 0.0, -0.0008, 1000.0, 1000000, 3, 5);
    planet_type p4 = createPlanet("p4", 300.0, 200.0, -0.0008, 0.0, 1000.0, 1000000, 4, 5);
    planet_type p5 = createPlanet("p3", 300.0, 400.0, 0.0008, 0.0, 1000.0, 1000000, 5, 5);

    head = createListHead();
    addNode(head, &p1);
    addNode(head, &p2);
    addNode(head, &p3);
    addNode(head, &p4);
    addNode(head, &p5);


    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me
    gtk_init(&argc, &argv); //Initialize GTK environment
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); //Create a new window which will serve as your top layer
    darea = gtk_drawing_area_new(); //Create draw area, which will be used under top layer window
    gtk_container_add(GTK_CONTAINER(window), darea); //add draw area to top layer window
    g_signal_connect(G_OBJECT(darea), "draw",
            G_CALLBACK(on_draw_event), NULL); //Connect callback function for the draw event of darea
    g_signal_connect(window, "destroy", //Destroy event, not implemented yet, altough not needed
            G_CALLBACK(gtk_main_quit), NULL);

    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); //Set position of window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600); //Set size of window
    gtk_window_set_title(GTK_WINDOW(window), "GTK window"); //Title
    gtk_widget_show_all(window); //Show window
    gtk_widget_add_tick_callback(darea, on_frame_tick, NULL, 1); //Add timer callback functionality for darea
    //GUI stuff, don't touch unless you know what you are doing, or if you talked to me


    Node * ptr = head->head;
    int id = 1;
    while (ptr != NULL)
    {
        threadCreate(threadTestFunc, id, ptr);
        ptr = ptr->next;
        id++;
    }


    threadCreate(getPlanets, 0, NULL);
    pthread_exit(NULL);

    gtk_main(); //Call gtk_main which handles basic GUI functionality
    return 0;
}
*/
