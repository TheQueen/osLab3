#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include "header.h"

#define PORT 5555

MsgList *head = NULL;                                 //Needs lots of mutex -_-
struct sockaddr_in remaddr;
socklen_t addrlen = sizeof(remaddr);
int fd;
char *server = "127.0.0.1";	/* change this to use a different server */
int sendPermission = 0;
int connectionId = 0;
int windowSize = 0;
int seqStart = 0;          //TODO: set this if I want
int connectionPhase = 0;
int wrong = 0; 
MsgList node;
pthread_mutex_t mutex;

clock_t timerStart = 0;
clock_t timerStop = 0;
clock_t roundTripTime = 0;
clock_t timer = 0;

int createSock();
void initSockReceiveOn(int fd, int port);
void initSockSendTo(int port);
void * connectionThread(void * arg);
void * sendThread(void * arg);
void * receiveThread(void * arg);
int errorCheck(DataHeader *buffer);

int main(int argc, char *argv[])
{
    pthread_t reader, writer;

    if((fd = createSock()) == 0)
    {
      printf("fd not created\n");
      exit(EXIT_FAILURE);
    }
    initSockReceiveOn(fd, 0);
    initSockSendTo(PORT);

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&reader, NULL, receiveThread, NULL);
    pthread_create(&writer, NULL, connectionThread, NULL);
    pthread_exit(NULL);

    sendPermission = 0;
    connectionId = 0;
    windowSize = 0;
    seqStart = 0;
    connectionPhase = 0;
    printf("Connection closed\n");

    return (EXIT_SUCCESS);
}

int createSock()
{
	int fd;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}
	return fd;
}
void initSockReceiveOn(int fd, int port)
{
	struct sockaddr_in myaddr;	/* our address */
	/* bind it to all local addresses and pick any port number */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		printf("bind failed");
		exit(EXIT_FAILURE);
	}
}
void initSockSendTo(int port)
{
	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(port);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
}

void * connectionThread(void *arg)
{
  DataHeader syn;
  DataHeader synack;
  DataHeader fin;
  DataHeader finack;
  MsgList *currentNode = NULL;

  ////////////////////////////////////SYN////////////////////////////////////////////////
  createDataHeader(0, 0, 0, 0, getCRC(strlen("SYN"), "SYN"), "SYN", &syn);

  while(connectionPhase == 0)
  {
    while (1)
    {
      if (pthread_mutex_trylock(&mutex))
      {
        timerStart = clock();
        pthread_mutex_unlock(&mutex);
        break;
      }
    }
    //Send syn to server
    if (sendto(fd, &syn, sizeof(DataHeader), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
    {
      printf("syn failed\n");
      exit(EXIT_FAILURE);
    }
    
    sleep(1);
  }

  ////////////////////////////////////SYNACKACK////////////////////////////////////////////
  //create SYNACK
  createDataHeader(1, connectionId, seqStart, windowSize, getCRC(strlen("SYNACK"), "SYNACK"), "SYNACK", &synack);

  while(connectionPhase == 1)
  {
    connectionPhase = 2;
    //Send synackack to server
    if (sendto(fd, &synack, sizeof(DataHeader), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
    {
      printf("syn failed\n");
      exit(EXIT_FAILURE);
    }
    
    //wait timer
    clock_t timer = clock() + roundTripTime;
    while (clock() < timer);
  }

  printf("\nConnected to receiver!\n");
  /////////////////////////////message sending/////////////////////////////////////////

	//head = (MsgList*)malloc(sizeof(MsgList));
  while (1)
  {
    if (pthread_mutex_trylock(&mutex))
    {
      head = createMessages(head, connectionId, seqStart+1, windowSize);
      pthread_mutex_unlock(&mutex);
      break;
    }
  }
	currentNode = head;
  while(head != NULL)
  {
    while (1)
    {
		//printf("while\n");
		fflush(stdout);
      if (pthread_mutex_trylock(&mutex))
      {
        if(sendPermission < windowSize && currentNode != NULL)
        {
			
          pthread_create(&currentNode->thread, NULL, sendThread, (void*)currentNode);
            sendPermission = sendPermission + 1; 
          currentNode = currentNode->next;
        }
        pthread_mutex_unlock(&mutex);
        break;
      }
    }
  }
  
  connectionPhase = 3;

  //////////////////////////closing connection//////////////////////////////////
  createDataHeader(3, connectionId, 0, windowSize, getCRC(strlen("FIN"), "FIN"), "FIN", &fin);
  node.sent = 0;
  node.acked = 0;
  node.data = &fin;
  node.next = NULL;
  pthread_create(&node.thread, NULL, sendThread, (void*)&node);
  pthread_join(node.thread, NULL);

  connectionPhase = 4;
  while (connectionPhase == 4)
  {
    if(connectionPhase == 5)
    {
        
        break;
    }
  }

  createDataHeader(4, connectionId, 0, windowSize, getCRC(strlen("FINACK"), "FINACK"), "FINACK", &finack);
  while(connectionPhase == 5)
  {
    
    connectionPhase = 6;
    //Send synackack to server
    if (sendto(fd, &finack, sizeof(DataHeader), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
    {
      printf("finack failed\n");
      exit(EXIT_FAILURE);
    }

    //wait timer
    clock_t timer = clock() + roundTripTime;
    while (clock() < timer);
	 //sleep(1);
  }
  return NULL;
}

void * sendThread(void * arg)
{
    int crc = ((MsgList*)arg)->data->crc; 
  int type = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	if (type != 0)
	{
		printf("Error in type\n");
	}
  while(((MsgList*)arg)->acked == 0)
  {
    while (1)
    {
        wrong = wrong +1; 
         if(((MsgList*)arg)->acked != 0)
        {
            break;
        }
      if (pthread_mutex_trylock(&mutex))
      {
        if((MsgList*)arg != NULL)
        {
            if((wrong % 3) == 0)
            {
                ((MsgList*)arg)->data->crc = 404; 
                printf("wrong crc in seq: %d\n", ((MsgList*)arg)->data->seq );
                fflush(stdout);
            }
            if((wrong % 10) == 0)
            {
                printf("msg \"lost\" seq: %d\n", ((MsgList*)arg)->data->seq ); 
            }
            else
            {
              if (sendto(fd, ((MsgList*)arg)->data, sizeof(DataHeader), 0, (struct sockaddr *)&remaddr, addrlen) < 0)
              {
                printf("send failed\n");
                exit(EXIT_FAILURE);
              }
            }
        }
        pthread_mutex_unlock(&mutex);
          ((MsgList*)arg)->data->crc = crc; 
        break;
      }
       
    }
    ((MsgList*)arg)->sent = 1;
    //wait timer
    clock_t timer = clock() + (roundTripTime * 4);
    while (clock() < timer);
	//sleep(1);
  }
  return NULL;
}

void * receiveThread(void * arg)
{
  int bytesReceived = 0;
  DataHeader buffer;

  while(connectionPhase < 6)
  {
	
    bytesReceived = recvfrom(fd, &buffer, sizeof(DataHeader), 0, (struct sockaddr *)&remaddr, &addrlen);
    //Add check for address that we received from
	  printf("flag: %d. seq: %d msg from recv: %s\n", buffer.flag, buffer.seq, buffer.data);
	  //printf("connectionPhase: %d\n", connectionPhase);
    if (bytesReceived > 0 && (calcError(buffer.crc, strlen(buffer.data), buffer.data)) == 0)
    {
      switch (buffer.flag)
      {
        case 0:
          //SYN - SHOULD NOT RECEIVE - DO NOTHING
          break;
        case 1:
          //SYNACK
          //if not connected then connect
          if(connectionPhase == 0)
          {
            while (1)
            {
              if (pthread_mutex_trylock(&mutex))
              {
                timerStop = clock();
                roundTripTime = (timerStop - timerStart) * 2;
                connectionPhase = 1;
                windowSize = buffer.windowSize;
                connectionId = buffer.id;
                pthread_mutex_unlock(&mutex);
                break;
              }
            }
            printf("Sender connected with id: %d and messages creates with window size: %d\n", connectionId, windowSize);
          }
          //receiver timer must have been triggered and our SYNACK must have been lost
          else if(connectionPhase == 2)
          {
            connectionPhase = 1;
          }
          break;
        case 2:
          //MSGACK
          //check connectionId if 0 then dont do stuff if not 0 do stuff
  			  //printf("msg ack goten \n");
  			  //fflush(stdout);
          if(connectionPhase == 2 && head != NULL)
          {
    			  //printf("yes \n");
    			  //fflush(stdout);
            while (1)
            {
      				//printf("blw \n");
      			  //fflush(stdout);
              if (pthread_mutex_trylock(&mutex))
              {
        				//printf("set ack \n buffer: %d\n data: %d\n", buffer.seq, head->data->seq);
        			  //fflush(stdout);
                if(buffer.seq < head->data->seq)
                {
                    //ack allready gotten
                    break;
                }
                setAck(head, buffer.seq, windowSize);
                head = removeFirstUntilNotAcked(head, &sendPermission);
                pthread_mutex_unlock(&mutex);
                break;
              }
            }
          }
          break;
        case 3:
          //FIN
          if(connectionPhase == 4)
          {
            connectionPhase = 5;
          }
          if (connectionPhase == 6)
          {
            connectionPhase = 5;
              return NULL;
          }
              
          break;
        case 4:
          //FINACK
          if(connectionPhase == 3)
          {
            //pthread_cancel(node.thread);
              node.acked = 1; 
               
          }
          break;
        default:
          break;
      }
    }
  }
  return NULL;
}
