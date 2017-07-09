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

#define PLANETIPC "/PlanetLab"
#define FNAME "/mapVars"
#define PLANETDEAD "/Deeeeead"
mqd_t mq;

ListHead * head;
int id = 1;

pthread_mutex_t addOrRemove;

int calc(planet_type * planet) {
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
        //printf("%s / %s \n", ptr->planet->name, planet->name);
        if (!strncmp(ptr->planet->name, planet->name, strlen(planet->name)))
        {

        }
        else {
            double x = ptr->planet->sx - planet->sx;
          //  printf("x: %f \n", x);
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
        return 0;
    }
    if (((planet->sx > 800) || (planet->sx < 0)) || ((planet->sy > 600) || (planet->sy < 0))) {
        planet->life = -1;
        return -1;
    }
    return 1;
}

void * handlePlanet(void * arg)
{
    planet_type * planet = (planet_type *) arg;

    while(planet->life > 0)
    {
        calc(planet);
        usleep(1000);

    }

    mqd_t tempmq;
    if (MQconnect(&tempmq, PLANETDEAD) == 0)
    {
        printf("Failed to connect to client!\n");
    }
    if(MQwrite(&tempmq, planet) != 0)
    {
        printf("could not write to client when dead\n");
    }


    while (1)
    {
        if (pthread_mutex_trylock(&addOrRemove))
        {
            int back = deleteNode (head, planet->name);
            printf("back: %d", back);
            pthread_mutex_unlock(&addOrRemove);
            break;
        }
    }



    pthread_exit(NULL);
}

void * getPlanets(void * arg)
{
    printf("getPlanets\n");
    fflush(stdout);
    planet_type * tempP;


    int bytes_read;

    if (!MQcreate(&mq, PLANETIPC))
    {
        printf("Failed to create server!\n");
        return NULL;
    }

    printf("created server!\n");
    fflush(stdout);

    while(1)
    {
        printf("while\n");
        fflush(stdout);
        tempP = (planet_type *) malloc(sizeof(planet_type ));
        //printf("kolla tempP\nsize planet_type = %d\nsize thempP = %d\n", sizeof(planet_type)), sizeof(*tempP);
        fflush(stdout);
        if(tempP == NULL)
        {
            printf("tempP == NULL\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        bytes_read = MQread(&mq, tempP);
        printf("MQread\n");
        fflush(stdout);

        if(bytes_read == -1)
        {
            printf("Error MQread: %s\n", strerror(errno));
            //return NULL;
        }
        else
        {

            pthread_mutex_lock(&addOrRemove);
            Node * p = addNode(head, *tempP);
            pthread_mutex_unlock(&addOrRemove);
            printf("Planet %d: %s\n", id, tempP->name);
            //tempP.pid = id;//ehhhhhhh ska vi göra såhär? hur vet clienten vilka planeter den ska få tillbaka?
            threadCreate(handlePlanet, id, p->planet);
            id++;
        }
        free(tempP);
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
    //mq = (mqd_t *) malloc(sizeof(mqd_t));
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
    pthread_exit(NULL);
    return 0;
}
