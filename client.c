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

#define PLANETIPC "/PlanetLabd"
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
GtkWidget * scrollWin;
GtkWidget * scrollWinForPlanetList;
GtkWidget * scrollWinForSentPlanet;

GtkWidget * sentPlanets;

GtkWidget * frameForLocal; 
GtkWidget * frameForSentPlanet; 

GtkWidget * tempWidg;
GtkWidget * tempWidgTwo; 


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
    printf("%d = %s\n", (int)pid, returnName);

    if (MQconnect(&serverHandle, PLANETIPC) == 0)
    {
        printf("Failed to connect to server!\n");
        return (EXIT_SUCCESS);
    }
    printf("mq connected\n");

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
    GtkTextIter iter; 
    gtk_text_buffer_get_iter_at_offset(msgBoxBuffer, &iter, 0);
     gtk_text_buffer_insert(msgBoxBuffer, &iter, "Plain text\n", -1);
    //gtk_widget_destroy(tempWidgTwo);
	tempWidgTwo = gtk_check_button_new_with_label("trying stuffffff");
	gtk_container_add(planetList, tempWidgTwo);//, -1);
	gtk_widget_show_all(window);
    
}

void testFuncTwo(GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active(widget) == TRUE) 
		{
			printf("clicked");
			fflush(stdout);
		}
		else
		{
			printf("unclicked");
			fflush(stdout); 
		}

}

void toggle_button_callback (GtkWidget *widget, gpointer   data)
{
    if (GTK_TOGGLE_BUTTON (widget) == TRUE) 
    {
        printf("unclicked");
		fflush(stdout);
    }
	else
	{
		printf("clicked");
		fflush(stdout); 
	}
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
    /*gboolean a = TRUE;
    planetList = gtk_tree_view_new(); // behöver fixas till
    scrollWinForPlanetList = gtk_scrolled_window_new(NULL,NULL);
    gtk_container_add(GTK_CONTAINER(scrollWinForPlanetList), planetList);
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
*/
    
    ///////////////////////////////////////PlanetList//////////////////////////////////////////////
    
    scrollWinForPlanetList = gtk_scrolled_window_new(NULL,NULL);
    planetList = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrollWinForPlanetList), planetList);
    
    tempWidg = gtk_check_button_new_with_label("remove this");
    //gtk_list_box_insert(planetList, tempWidg, -1); 
    
    gtk_list_box_insert(planetList, tempWidg, -1);
    
    frameForLocal = gtk_frame_new("Local Planets"); 
    gtk_container_add(GTK_CONTAINER(frameForLocal), scrollWinForPlanetList);
    gtk_widget_set_size_request(frameForLocal, 180, 200); 
    
    
    //////////////////////////////////////////Sent planets/////////////////////////////////////////
    
    scrollWinForSentPlanet = gtk_scrolled_window_new(NULL,NULL);
    sentPlanets = gtk_list_box_new();
    frameForSentPlanet = gtk_frame_new("Sent Planets");
    
    gtk_container_add(GTK_CONTAINER(scrollWinForSentPlanet), sentPlanets);
    gtk_container_add(GTK_CONTAINER(frameForSentPlanet), scrollWinForSentPlanet);
    
    gtk_widget_set_size_request(frameForSentPlanet, 180, 200);
    
    ////////////////////////////////////////////MsgBox/////////////////////////////////////////////
    msgBoxBuffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(msgBoxBuffer, "hello", 5);
    //denna funkar nu
    msgBox = gtk_text_view_new();

    gtk_text_view_set_buffer(msgBox, msgBoxBuffer);

    scrollWin = gtk_scrolled_window_new(NULL,NULL);
    gtk_container_add(GTK_CONTAINER(scrollWin), msgBox);
    
    gtk_widget_set_size_request(scrollWin, 270, 250);

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
    g_signal_connect(saveButton, "clicked", G_CALLBACK(save), NULL);

    g_signal_connect(addButton, "clicked", G_CALLBACK(startAddWindow), NULL);
    g_signal_connect(sendButton, "clicked", G_CALLBACK(sendPlan), NULL);


    //lägger in fixed i window
    gtk_container_add(GTK_CONTAINER(window), fixed);

    //sätter var alla saker ska vara i fixed som är i window, så de hamnar indirekt i window
    gtk_fixed_put(GTK_FIXED (fixed), loadButton, 440, 340);
    gtk_fixed_put(GTK_FIXED (fixed), saveButton, 440, 400);
    gtk_fixed_put(GTK_FIXED (fixed), sendButton, 440, 450);
    gtk_fixed_put(GTK_FIXED (fixed), addButton, 80, 450);
    gtk_fixed_put(GTK_FIXED (fixed), loadName, 250, 340);
    gtk_fixed_put(GTK_FIXED (fixed), saveName, 250, 400);
    gtk_fixed_put(GTK_FIXED(fixed), frameForLocal, 10, 10); //-funkar ej
    gtk_fixed_put(GTK_FIXED(fixed), loadLabel, 250, 320);
    gtk_fixed_put(GTK_FIXED(fixed), saveLabel, 250, 380);
    gtk_fixed_put(GTK_FIXED(fixed), lPlanetNum, 40, 455);
    gtk_fixed_put(GTK_FIXED(fixed), scrollWin, 200, 10);
    gtk_fixed_put(GTK_FIXED(fixed), frameForSentPlanet, 10, 220);
    
    //set tab
    gtk_entry_set_tabs(loadName, tab);
    gtk_entry_set_tabs(saveName, tab);


    //gör alla widgetar som finns i window synliga
    gtk_widget_show_all(window);

}

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
	GtkAdjustment * Adj = gtk_adjustment_new(0, 0, 800, 1, 1, 1);
    addStartPosLabel = gtk_label_new("Start position: ");
    addStartX = gtk_spin_button_new (Adj, 2, 3);
    gtk_entry_set_text(addStartX, "X-pos");
	Adj = gtk_adjustment_new(0, 0, 600, 1, 1, 1);
    addStartY = gtk_spin_button_new (Adj, 2, 3);
    gtk_entry_set_text(addStartY, "Y-pos");
    gtk_fixed_put(GTK_FIXED (addFixed), addStartPosLabel, 20, 80);
    gtk_fixed_put(GTK_FIXED (addFixed), addStartX, 20, 100);
    gtk_fixed_put(GTK_FIXED (addFixed), addStartY, 20, 130);

    //Movment section
    addMovementLabel = gtk_label_new("Movement: ");
	Adj = gtk_adjustment_new(0, -1, 2, 0.01, 1, 1);
    addMovementX = gtk_spin_button_new (Adj, 2, 4);
    gtk_entry_set_text(addMovementX, "X-pos");
	Adj = gtk_adjustment_new(0, -1, 2, 0.01, 1, 1);
    addMovementY =  gtk_spin_button_new (Adj, 2, 4);
    gtk_entry_set_text(addMovementY, "Y-pos");
    gtk_fixed_put(GTK_FIXED (addFixed), addMovementLabel, 20, 170);
    gtk_fixed_put(GTK_FIXED (addFixed), addMovementX, 20, 190);
    gtk_fixed_put(GTK_FIXED (addFixed), addMovementY, 20, 220);

    //Life section
    addLifeLabel = gtk_label_new("Life: ");
	Adj = gtk_adjustment_new(0, 0, 10000000, 1, 1, 1);
    addLifeInput = gtk_spin_button_new (Adj, 2, 3);
    gtk_fixed_put(GTK_FIXED (addFixed), addLifeLabel, 20, 260);
    gtk_fixed_put(GTK_FIXED (addFixed), addLifeInput, 20, 280);

    //radius section
    addRadiusLabel = gtk_label_new("Radius: ");
	Adj = gtk_adjustment_new(0, 0, 10, 1, 1, 1);
    addRadiusInput = gtk_spin_button_new (Adj, 2, 3);
    gtk_fixed_put(GTK_FIXED (addFixed), addRadiusLabel, 20, 320);
    gtk_fixed_put(GTK_FIXED (addFixed), addRadiusInput, 20, 340);

    //mass section
    addMassLabel = gtk_label_new("Mass: ");
	Adj = gtk_adjustment_new(0, 0, 10000000, 1, 1, 1);
    addMassInput = gtk_spin_button_new (Adj, 2, 3);
    gtk_fixed_put(GTK_FIXED (addFixed), addMassLabel, 20, 380);
    gtk_fixed_put(GTK_FIXED (addFixed), addMassInput, 20, 400);

    //addButton
    finalAddButton = gtk_button_new_with_label("Add");
    g_signal_connect(finalAddButton, "clicked", G_CALLBACK(add), NULL);
    gtk_fixed_put(GTK_FIXED (addFixed), finalAddButton, 150, 450);


    gtk_widget_show_all(addWindow);
}

//Observer funcs
void onCheck(GtkWidget *widget)
{
    //Denna är nog Stinas
    //TODO: mutex stuff
    //ändra utseende
    //ändra int checked
	
	char * name = gtk_button_get_label(widget); 
	int check = 0;
    PlanetDisplayList *temp = NULL;
    temp = displayListHead;

    
//byt mot find Planet
    while(temp != NULL)
    {
		if(temp == NULL)
		{
			printMsg("Error: No planets in list!\n");
			return;
		}
        if (strcmp(temp->planet->name, name) == 0)
        {
          	if (GTK_TOGGLE_BUTTON (widget) == TRUE)
			{
				temp->checked = 1;
			}
			else
			{
				temp->checked = 0; 
			}
        }

        temp = temp->next;
    }
	//find displaylistnode with name
}

void printMsg(char * msg)
{
    //TODO: Denna är nog Stinas
    //ska skiriva i meddelanderuta egentligen
    printf("%s", msg);
}
void load ()
{
    planet_type temp;
    gchar * filename = gtk_entry_get_text (loadName);
    printf ("%s\n", filename);

    FILE *fp;
    fp = fopen(filename, "rb");



    while(1)
    {
      temp = (planet_type*)malloc(sizeof(planet_type));
      if(fread(temp, sizeof(planet_type), 1, fp))
      {
        printf ("%s\n", temp->name);
        tempDisp = createDisplayListNode(temp);
        //TODO: Mutex stuff
        displayListHead = addFirstToDisplayList(displayListHead, tempDisp);
      }
      else
      {
        free(temp);
        break;
      }
    }



    if((fclose(fp )) != 0)
    {
        printf("Error: couldn´t close file correctly\n");
        exit(EXIT_FAILURE);
    }
}
void save()
{
    //TODO: mutex stuff
    gchar * filename = gtk_entry_get_text (saveName);
    printf ("%s\n", filename);

    strcat(filename, ".bin");

    FILE *fp;
    fp = fopen(filename, "wb");

    //Make new pointer to use for saving and set it to first planet in the list
    PlanetDisplayList *temp = NULL;
    temp = displayListHead;

    //I just save the planets so that when I load, I don´t get a checked box on every loaded planet
    while(temp != NULL)
    {
        if (temp->checked)

        {
           fwrite(temp->planet, sizeof(planet_type), 1, fp);
        }
        temp = temp->next;
    }
    if((fclose(fp )) != 0)
    {
        printf("Error: couldn´t close file correctly\n");
        exit(EXIT_FAILURE);
    }
}
//TODO: Kolla: behöver vi denna funktion? Är inte den någon annan som redan finns?
void addObsever()
{
    //öppna add-fonster
    //return 404
}
//TODO: Kolla: Vad är detta för funktion? Jag fattar inte... är det inte samma fråga som på funktionen ovan?
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

	
    const char * name = gtk_entry_get_text (addNameInput);
    const double xpos = gtk_spin_button_get_value (addStartX);
    const double ypos = gtk_spin_button_get_value (addStartY);
    const double movx = gtk_spin_button_get_value (addMovementX);
    const double movy = gtk_spin_button_get_value (addMovementY);
    const double life = gtk_spin_button_get_value (addLifeInput);
    const int radius = gtk_spin_button_get_value_as_int (addRadiusInput);
    const double mass = gtk_spin_button_get_value (addMassInput);

    printf ("name: %s\n", name);

	
    planet_type newPlanet = createPlanet(name, xpos, ypos, movx, movy, mass, life, returnName, radius);
    //TODO: mutex stuff

    displayListHead = addFirstToDisplayList(displayListHead, &newPlanet);
	displayListHead->check = gtk_check_button_new_with_label(name);
	gtk_container_add(planetList, displayListHead->check);//, -1);
	g_signal_connect(displayListHead->check, "toggled",G_CALLBACK(onCheck), NULL); 
	gtk_widget_show_all(window);

}

////////////////////////////Övrigt///////////////////////////////////////////////////////////////////////////
void sendPlan()
{
    //sendPlanet(returnName);
    //get planets from list
    //send planet
    //print msg
    //TODO: mutex stuff
    planet_type p[5];

    p[0] = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, returnName, 5);
    p[1] = createPlanet("p2", 200.0, 300.0, 0.0, 0.0008, 1000.0, 100000000, returnName, 5);
    p[2] = createPlanet("p3", 400.0, 300.0, 0.0, -0.0008, 1000.0, 1000000, returnName, 5);
    p[3] = createPlanet("p4", 300.0, 200.0, -0.0008, 0.0, 1000.0, 1000, returnName, 5);
    p[4] = createPlanet("p5", 300.0, 400.0, 0.0008, 0.0, 1000.0, 1000000, returnName, 5);

    printf("%s\n", p[0].name);

    displayListHead = addFirstToDisplayList(displayListHead, createDisplayListNode(&p[0]));
    displayListHead = addFirstToDisplayList(displayListHead, createDisplayListNode(&p[1]));
    displayListHead = addFirstToDisplayList(displayListHead, createDisplayListNode(&p[2]));
    displayListHead = addFirstToDisplayList(displayListHead, createDisplayListNode(&p[3]));
    displayListHead = addFirstToDisplayList(displayListHead, createDisplayListNode(&p[4]));

    printf("%s\n", displayListHead->planet->name);

    int check = 0;
    PlanetDisplayList *temp = NULL;
    temp = displayListHead;
    // check = MQwrite(&serverHandle, &p[0]);
    // if(check != 0)
    // {
    //     printf("could not write to mailbox: %s\n", strerror(errno));
    //     exit(EXIT_FAILURE);
    // }
    if(temp == NULL)
    {
        printMsg("Error: No planets in list!\n");
        return;
    }

    while(temp != NULL)
    {
        //TODO: ta bort kommentaren
        if (/*temp->checked*/1)
        {

          check = MQwrite(&serverHandle, temp->planet);

          if(check != 0)
          {
              printf("could not write to mailbox: %s\n", strerror(errno));
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
    if (!MQcreate(&mq, returnName))
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
            displayListHead = removePlanet(displayListHead, tempP.name);
            //sleep(3);
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
    planet_type planet[5];
    //planet[0] = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, 1, 5);
    //planet[1] = createPlanet("p2", 200.0, 300.0, 0.0, 0.0008, 1000.0, 100000000, 2, 5 );
    planet[0] = createPlanet("p1", 300.0, 300.0, 0.0, 0.0, 10000000.0, 300000000, arg, 5);
    planet[1] = createPlanet("p2", 200.0, 300.0, 0.0, 0.0008, 1000.0, 100000000, arg, 5);
    planet[2] = createPlanet("p3", 400.0, 300.0, 0.0, -0.0008, 1000.0, 1000000, arg, 5);
    planet[3] = createPlanet("p4", 300.0, 200.0, -0.0008, 0.0, 1000.0, 1000, arg, 5);
    planet[4] = createPlanet("p5", 300.0, 400.0, 0.0008, 0.0, 1000.0, 1000000, arg, 5);
    int check = 0;

    sleep(2);

    for(int i=0; i<5; i++)
    {
        check = MQwrite(&serverHandle, &planet[i]);
        if(check != 0)
        {
            printf("could not write to mailbox\n");
            break;
        }
    }
    //printf("planets sent\n");
    return NULL;
}

void * readDead(void * arg)
{
    mqd_t mq;
    int count = 0;
    planet_type tempP;
    int bytes_read;

    if (!MQcreate(&mq, arg))
    {
        printf("Failed to create server!\n");
        return NULL;
    }
    printf("created readDead!\n");

    while(count < 5)
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
        }
    }
    if(MQclose(&mq, arg) == 1)
    {
        printf("mailbox was successfully closed\n");
    }
    return NULL;
}



// void * readDead(void * arg)
// {
//     int count = 0;
//     planet_type tempP;
//     int bytes_read;
//     //pthread_t block;
//     if (!MQcreate(&mq, PLANETDEAD))
//     {
//         printf("Failed to create server!\n");
//         return NULL;
//     }
//
//     while(1)
//     {
//         bytes_read = MQread(&mq, &tempP);
//         if(bytes_read != -1)
//         {
//             switch(tempP.life)
//             {
//                 case  0: printf("Planet %s died of old age\n", tempP.name); break;
//                 case -1: printf("Planet %s disappeared\n", tempP.name); break;
//                 default: printf("Eeeeeeehhhhrror\n"); break;
//             }
//             count++;
//             sleep(20);
//             //pthread_create(&block, NULL, readDead/*Felfunktion*/, NULL);
//
//         }
//         if(count == 5)
//         {
//             printf("I should break");
//         }
//
//
//     }
//     if(MQclose(&mq, PLANETDEAD) == 1)
//     {
//         printf("mailbox was successfully closed\n");
//     }
//     return NULL;
// }


// pthread_t reader = 0;
// pthread_t writer = 0;
// pid_t pid;
// pid = getpid();
// char mqReturnName[30];
// mqReturnName[0] = '/';
// char car[20];
// snprintf(car, 10, "%d", pid);
// printf("%d = %s\n", (int)pid, mqReturnName);
// strcat(mqReturnName, car);
//
//
// if (MQconnect(&serverHandle, PLANETIPC) == 0)
// {
//     printf("Failed to connect to server!\n");
//     return (EXIT_SUCCESS);
// }
// printf("mq connected\n");
//
// threadCreate(sendPlanet, writer, mqReturnName);
// threadCreate(readDead, reader, mqReturnName);
// pthread_exit(NULL);
//
// return (EXIT_SUCCESS);
