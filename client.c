#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <pthread.h>
//#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include "wrapper.h"
#include "list.h"
#include "planetDisplayList.h"

#define PLANETIPC "/PlanetLab"
#define PLANETDEAD "/Deeeeead" //TODO: Fix this to be unique(also in server) -- Jag tror jag kommer ihåg att jakob sa att vi behövde fixa det till nästa lab

char * returnName;

mqd_t serverHandle;
mqd_t mq;
////////////////////////////////GTK/////////////////////////////////////////////
//TODO: Stina? add number of active planets and dot or something to indicate which planets are active
//Alla widgetar så att vi kan komma åt dem i andra funktioner ^^
GtkWidget * window;
GtkWidget * fixed;
GtkWidget * msgBox;
GtkWidget * loadLabel;
GtkWidget * loadName;
GtkWidget * loadButton;
GtkWidget * saveLabel;
GtkWidget * saveName;
GtkWidget * saveButton;
GtkWidget * lPlanetNum;
GtkWidget * addButton;
GtkWidget * sendButton;
GtkWidget * planetList;
GtkTextBuffer * msgBoxBuffer; //buffer för msgBox
GtkApplication * app; //applicationen

//widgetar och stuff för startAddWindow
GtkWidget * addWindow;
GtkWidget * finalAddButton;
GtkWidget * addNameLabel;
GtkWidget * addNameInput;
GtkWidget * addStartPosLabel;
GtkWidget * addStartX;
GtkWidget * addStartY;
GtkWidget * addMovementLabel;
GtkWidget * addMovementX;
GtkWidget * addMovementY;
GtkWidget * addLifeLabel;
GtkWidget * addLifeInput;
GtkWidget * addRadiusLabel;
GtkWidget * addRadiusInput;
GtkWidget * addMassLabel;
GtkWidget * addMassInput;
GtkWidget * addFixed;
PangoTabArray * tab; //sak för att kunna tabba igenom alla knappar och inputrutor

//////////////////////////////stuff//////////////////////////////////////////////
PlanetDisplayList * displayListHead; //NYAListan med alla inlagda planeter + annatgtkskit
int activePlanets = 0;

//GTK
void testFunc();
void startObserver (GtkApplication * app, gpointer uData);
void startAddWindow(GtkApplication * app, gpointer uData);

//Observer
void onCheck();
void onUnCheck();
void printMsg(char *msg);
void load();
void save();
void addObsever(); //Eh behöver vi denna? Den är väl lika med startAddWindow?
void sendOpen(); //Vad är detta för funktion?

//Add
void add();
//övrigt
void sendPlan();
void readDeadPlan();
void updateList();      //Vill vi ha denna funktion?

////////////Lab2//////////////////////////////////////////////////
void * sendPlanet(void * arg);
void * readDead(void * arg);
//////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    pthread_t reader = 0;
    displayListHead = NULL;
    int status;

    pid_t pid;
    pid = getpid();
    char mqReturnName[30];
    mqReturnName[0] = '/';
    char car[20];
    snprintf(car, 10, "%d", pid);
    strcat(mqReturnName, car);
    returnName = mqReturnName;

    //printf("%d = %s\n", (int)pid, mqReturnName);

    //initTab
    tab = pango_tab_array_new(30, TRUE);

    threadCreate(readDead, reader, mqReturnName);

    //skapar appliacationen
    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(startObserver), NULL);
    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    //return (EXIT_SUCCESS);
    return status;
}

//skräp funktion
void testFunc()
{
    printf ("haha");
}

void startObserver (GtkApplication * app, gpointer uData)
{
    //skapar fönstret
    window = gtk_application_window_new (app);
    //sätter rubriken på fönstret
    gtk_window_set_title(GTK_WINDOW (window), "Observer");
    //sätter storleken på fönstret
    gtk_window_set_default_size(GTK_WINDOW(window),500,500);

    //skapar fixed variabeln
    //- den gör så att man kan bestämma var saker ska vara
    fixed = gtk_fixed_new();

    //denna kod är inte klar har inte fått den att funka än  så återkommer
    //när den funkar
    gboolean a = TRUE;
    planetList = gtk_tree_view_new(); // behöver fixas till
    gtk_tree_view_set_headers_visible(planetList, a);


    //samma med denna
    GtkTreeViewColumn * checkColumn = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(checkColumn, "Check");

    GtkTreeViewColumn * planetsColumn = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(planetsColumn, "Planets");

    GtkTreeViewColumn * localColumn = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(localColumn, "Local");

    gtk_tree_view_append_column(planetList, checkColumn);
    gtk_tree_view_append_column(planetList, planetsColumn);
    gtk_tree_view_append_column(planetList, localColumn);

    //och denna//
    msgBoxBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(msgBoxBuffer, "hello", 5);
    //denna oxå
    msgBox = gtk_text_view_new();

    gtk_text_view_set_buffer(msgBox, msgBoxBuffer);

    gtk_widget_set_size_request(msgBox, 270, 250);
    gtk_widget_set_size_request(planetList, 180, 400);

    //skapar alla statiska texter
    loadLabel = gtk_label_new("Load planets");
    saveLabel = gtk_label_new("Save planets");
    lPlanetNum = gtk_label_new("0");

    //skapar textfälten
    loadName = gtk_entry_new();
    saveName = gtk_entry_new();

    //sätter den temporära texten i textfälten
    gtk_entry_set_text(loadName, "Enter filename");
    gtk_entry_set_text(saveName, "Enter filename");

    //skapar alla knapparna och sätter en text på dem
    loadButton = gtk_button_new_with_label("load");
    sendButton = gtk_button_new_with_label("send");
    saveButton = gtk_button_new_with_label("save");
    addButton = gtk_button_new_with_label("add");

    //sätter vad som ska hända när man klickar på knappen
    //tex load knappen som kallar load funktionen
    g_signal_connect(loadButton, "clicked", G_CALLBACK(load), NULL);
    g_signal_connect(sendButton, "clicked", G_CALLBACK(sendOpen), NULL);
    g_signal_connect(saveButton, "clicked", G_CALLBACK(save), NULL);
    g_signal_connect(addButton, "clicked", G_CALLBACK(startAddWindow), NULL);

    //lägger in fixed i window
    gtk_container_add(GTK_CONTAINER(window), fixed);

    //sätter var alla saker ska vara i fixed som är i window, så de hamnar indirekt i window
    gtk_fixed_put(GTK_FIXED (fixed), loadButton, 440, 340);
    gtk_fixed_put(GTK_FIXED (fixed), saveButton, 440, 400);
    gtk_fixed_put(GTK_FIXED (fixed), sendButton, 440, 450);
    gtk_fixed_put(GTK_FIXED (fixed), addButton, 80, 450);
    gtk_fixed_put(GTK_FIXED (fixed), loadName, 250, 340);
    gtk_fixed_put(GTK_FIXED (fixed), saveName, 250, 400);
    gtk_fixed_put(GTK_FIXED(fixed), planetList, 10, 10); //-funkar ej
    gtk_fixed_put(GTK_FIXED(fixed), loadLabel, 250, 320);
    gtk_fixed_put(GTK_FIXED(fixed), saveLabel, 250, 380);
    gtk_fixed_put(GTK_FIXED(fixed), lPlanetNum, 40, 455);
    gtk_fixed_put(GTK_FIXED(fixed), msgBox, 200, 10);

    //set tab
    gtk_entry_set_tabs(loadName, tab);
    gtk_entry_set_tabs(saveName, tab);


    //gör alla widgetar som finns i window synliga
    gtk_widget_show_all(window);

}

/*GtkWidget * addWindow;
GtkWidget * finalAddButton;
GtkWidget * addNameLabel;
GtkWidget * addNameInput;
GtkWidget * addStartPosLabel;
GtkWidget * addStartX;
GtkWidget * addStartY;
GtkWidget * addMovementLabel;
GtkWidget * addMovementX;
GtkWidget * addMovementY;
GtkWidget * addLifeLabel;
GtkWidget * addLifeInput;
GtkWidget * addRadiusLabel;
GtkWidget * addRadiusInput;
GtkWidget * addMassLable;
GtkWidget * addMassInput; */

void startAddWindow(GtkApplication * app, gpointer uData)
{
    //Window
    addWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (addWindow), "Add a planet");
    gtk_window_set_default_size(GTK_WINDOW (addWindow), 200,500);

    //Fixed
    addFixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(addWindow), addFixed);

    //Name section
    addNameLabel = gtk_label_new("Name");
    addNameInput = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED (addFixed), addNameLabel, 20, 20);
    gtk_fixed_put(GTK_FIXED (addFixed), addNameInput, 20, 40);
    gtk_entry_set_tabs(addNameInput, tab);

    //StartPos section
    addStartPosLabel = gtk_label_new("Start position: ");
    addStartX = gtk_entry_new();
    gtk_entry_set_text(addStartX, "X-pos");
    addStartY = gtk_entry_new();
    gtk_entry_set_text(addStartY, "Y-pos");
    gtk_fixed_put(GTK_FIXED (addFixed), addStartPosLabel, 20, 80);
    gtk_fixed_put(GTK_FIXED (addFixed), addStartX, 20, 100);
    gtk_fixed_put(GTK_FIXED (addFixed), addStartY, 20, 130);

    //Movment section
    addMovementLabel = gtk_label_new("Movement: ");
    addMovementX = gtk_entry_new();
    gtk_entry_set_text(addMovementX, "X-pos");
    addMovementY = gtk_entry_new();
    gtk_entry_set_text(addMovementY, "Y-pos");
    gtk_fixed_put(GTK_FIXED (addFixed), addMovementLabel, 20, 170);
    gtk_fixed_put(GTK_FIXED (addFixed), addMovementX, 20, 190);
    gtk_fixed_put(GTK_FIXED (addFixed), addMovementY, 20, 220);

    //Life section
    addLifeLabel = gtk_label_new("Life: ");
    addLifeInput = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED (addFixed), addLifeLabel, 20, 260);
    gtk_fixed_put(GTK_FIXED (addFixed), addLifeInput, 20, 280);

    //radius section
    addRadiusLabel = gtk_label_new("Radius: ");
    addRadiusInput = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED (addFixed), addRadiusLabel, 20, 320);
    gtk_fixed_put(GTK_FIXED (addFixed), addRadiusInput, 20, 340);

    //mass section
    addMassLabel = gtk_label_new("Mass: ");
    addMassInput = gtk_entry_new();
    gtk_fixed_put(GTK_FIXED (addFixed), addMassLabel, 20, 380);
    gtk_fixed_put(GTK_FIXED (addFixed), addMassInput, 20, 400);

    //addButton
    finalAddButton = gtk_button_new_with_label("Add");
    g_signal_connect(finalAddButton, "clicked", G_CALLBACK(add), NULL);
    gtk_fixed_put(GTK_FIXED (addFixed), finalAddButton, 150, 450);


    gtk_widget_show_all(addWindow);
}


//Observer funcs
void onCheck()
{
    //Denna är nog Stinas
    //TODO: mutex stuff
    //ändra utseende
    //ändra int checked
}
void onUnCheck()
{
    //Denna är nog Stinas
    //TODO: mutex stuff
    //ändra utseende
    //ändra int checked
}
void printMsg(char *msg)
{
    //TODO: Denna är nog Stinas
    //ska skiriva i meddelanderuta egentligen
    printf("%s", msg);
}
void load ()
{
    planet_type *temp;
    gchar * filename = gtk_entry_get_text (loadName);
    printf ("%s\n", filename);
    strcat(filename, ".bin");

    FILE *fp;
    fp = fopen(filename, "rb");

    if(fp == NULL)
    {
      //TODO: put message in messagebox
      printf("File could not be opened\n");
      return;
    }

    while(1)
    {
      temp = (planet_type*)malloc(sizeof(planet_type));
      if(fread(temp, sizeof(planet_type), 1, fp))
      {
        printf ("%s\n", temp->name);
        //TODO: Mutex stuff
        displayListHead = addfirstToDisplayList(displayListHead, temp);
      }
      else
      {
        free(temp);
        break;
      }
    }


    fclose(fp);
}
void save()
{
    //TODO: mutex stuff
    gchar * filename = gtk_entry_get_text (saveName);
    printf ("%s\n", filename);

    strcat(filename, ".bin");

    FILE *fp;
    fp = fopen(filename, "wb");

    if(fp == NULL)
    {
      //TODO: put message in messagebox
      printf("File could not be opened\n");
      return;
    }

    //Make new pointer to use for saving and set it to first planet in the list
    PlanetDisplayList *temp = NULL;
    temp = displayListHead;

    //I just save the planets so that when I load, I don´t get a checked box on every loaded planet
    while(temp != NULL)
    {
        if (/*temp->checked*/1)
        {
           fwrite(temp->planet, sizeof(planet_type), 1, fp);
        }
        temp = temp->next;
    }
    fclose(fp);
}
//TODO: Stina Kolla: behöver vi denna funktion? Är inte den någon annan som redan finns?
void addObsever()
{
    //öppna add-fonster
    //return 404
}
//TODO: Stina Kolla: Vad är detta för funktion? Jag fattar inte... är det inte samma fråga som på funktionen ovan?
void sendOpen()
{
    //create thread send planet
    //Efter tillbaka:
    //ändra checkboxes
    //töm listan
}

//Add func
void add()
{
    //get input
    //make planet
    //add planet to list?
    //add to displaylist

    gchar * name = gtk_entry_get_text (addNameInput);
    gchar * xpos = gtk_entry_get_text (addStartX);
    gchar * ypos = gtk_entry_get_text (addStartY);
    gchar * movx = gtk_entry_get_text (addMovementX);
    gchar * movy = gtk_entry_get_text (addMovementY);
    gchar * life = gtk_entry_get_text (addLifeInput);
    gchar * radius = gtk_entry_get_text (addRadiusInput);
    gchar * mass = gtk_entry_get_text (addMassInput);

    printf ("%s\n", returnName);
    //TODO: set these with correctly casted values so that new planet can be created
    //Haven´t checked if this code works
    double xposp = atof(xpos);
    double yposp = atof(ypos);
    double xVp = atof(movx);
    double yVp = atof(movy);
    double massp = atof(mass);
    int lifep = atoi(life);
    int rp = atoi(radius);
    planet_type* newPlanet = (planet_type*)malloc(sizeof(planet_type));
    *newPlanet = createPlanet(name, xposp, yposp, xVp, yVp, massp, lifep, returnName, rp);
    PlanetDisplayList *temp = createDisplayListNode(newPlanet);
    //TODO: mutex stuff
    displayListHead = addfirstToDisplayList(displayListHead, temp);
    //TODO: Stina? uppdateGraphical interface
}

////////////////////////////Övrigt///////////////////////////////////////////////////////////////////////////
void sendPlan()
{
    //get planets from list
    //send planet
    //print msg
    //TODO: mutex stuff
    int check = 0;
    PlanetDisplayList *temp = NULL;
    temp = displayListHead;

    if(temp == NULL)
    {
        printMsg("Error: No planets in list!\n");
        return;
    }

    while(temp != NULL)
    {
        if (temp->checked)
        {
          check = MQwrite(&serverHandle, &temp->planet);

          if(check != 0)
          {
              printf("could not write to mailbox\n");
              exit(EXIT_FAILURE);
          }
          activePlanets++;
          temp->state = 1; //active
          //TODO: Stina also change list icon so that it is active
        }

        temp = temp->next;
    }
    printMsg("planets sent\n");
}

void readDeadPlan()
{
    planet_type tempP;
    int bytes_read;
    if (!MQcreate(&mq, PLANETDEAD))
    {
        printf("Failed to create message queue!\n");
        return;
    }

    while(1)
    {
        bytes_read = MQread(&mq, &tempP);
        if(bytes_read != -1)
        {
            switch(tempP.life)
            {
                //TODO: write in msgbox
                case  0: printf("Planet %s died of old age\n", tempP.name); break;
                case -1: printf("Planet %s disappeared\n", tempP.name); break;
                default: printf("Eeeeeeehhhhrror\n"); break;
            }
            activePlanets--;
            //TODO: mutex stuff for remove from active planets
            //TODO: remove planet from list
            sleep(3);
        }
    }

    if(MQclose(&mq, PLANETDEAD) == 1)
    {
        printf("mailbox was successfully closed\n");
    }
}
//Vill vi ha denna kanske?
void updateList()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lab2
void * sendPlanet(void * arg)
{
    planet_type planet[6];
    planet[0] = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, arg, 5);
    planet[1] = createPlanet("p2", 200.0, 300.0, 0.0, 0.0008, 1000.0, 100000000, arg, 5 );
    planet[2] = createPlanet("p3", 400.0, 300.0, 0.0, -0.0008, 1000.0, 1000000, arg, 5);
    planet[3] = createPlanet("p4", 300.0, 200.0, -0.0008, 0.0, 1000.0, 1000, arg, 5);
    planet[4] = createPlanet("p5", 300.0, 400.0, 0.0008, 0.0, 1000.0, 1000000, arg, 5);
    planet[5] = createPlanet("p6", 300.0, 400.0, 0.9, 0.0, 1000.0, 1000000, arg, 5);

    int check = 0;

    for(int i=0; i<6; i++)
    {

        check = MQwrite(&serverHandle, &planet[i]);
        printf("written to mq\n");

        if(check != 0)
        {
            printf("could not write to mailbox\n");
            break;
        }
    }
    return NULL;

}

void * readDead(void * arg)
{
    int count = 0;
    planet_type tempP;
    int bytes_read;
    //pthread_t block;
    if (!MQcreate(&mq, PLANETDEAD))
    {
        printf("Failed to create server!\n");
        return NULL;
    }

    while(1)
    {
        bytes_read = MQread(&mq, &tempP);
        if(bytes_read != -1)
        {
            switch(tempP.life)
            {
                case  0: printf("Planet %s died of old age\n", tempP.name); break;
                case -1: printf("Planet %s disappeared\n", tempP.name); break;
                default: printf("Eeeeeeehhhhrror\n"); break;
            }
            count++;
            sleep(20);
            //pthread_create(&block, NULL, readDead/*Felfunktion*/, NULL);

        }
        if(count == 5)
        {
            printf("I should break");
        }


    }
    if(MQclose(&mq, PLANETDEAD) == 1)
    {
        printf("mailbox was successfully closed\n");
    }
    return NULL;
}


    //pthread_t reader, writer;

    /*int i;
    pid_t pid;
    pid = getpid();
    char mqReturnName[30];
    mqReturnName[0] = '/';
    char car[20];
    snprintf(car, 10, "%d", pid);
    strcat(mqReturnName, car);

    //printf("%d = %s\n", (int)pid, mqReturnName);

    if (MQconnect(&serverHandle, PLANETIPC) == 0)
    {
            printf("Failed to connect to server!\n");
            return (EXIT_SUCCESS);
    }
    */
    //printf("created server!\n");
    //threadCreate(sendPlanet, writer, mqReturnName);
    //threadCreate(readDead, reader, mqReturnName);
    //pthread_exit(NULL);
