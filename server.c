/**
 * @file server.c
 * @author Pedro Cabral
 * @brief
 * @version 0.1
 * @date 2022-04-14
 *
 * @copyright Copyright (c) 2022
 *
 * I got inspiration from the webpage
 * https://www.geeksforgeeks.org/udp-server-client-implementation-c/
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*multithreading*/
#include <pthread.h>

#define MAXLINE 1024
#define DELAY 2*1000*1000
#define MAXCLIENTS 3

typedef struct udp_info{

  int sockfd;
  char buffer[MAXLINE+3];
  int nbytes;

  struct sockaddr_in client_addr;
  int len;
  int is_addr_valid;

  pthread_mutex_t *lock;

} udp_info;

void *echo_forever(int *sockfd_ptr);
void *send_messages(udp_info *info);
void *listen_messages(udp_info *info);

pthread_mutex_t lock[MAXCLIENTS];

int main(int argc, char *argv[]) {

  /* checking command line arguments */
  if (argc != 2) {
    printf("You should pass exactly one argument to the program: PORT_NUMBER\n");
    exit(-1);
  }

  int port;
  if (sscanf(argv[1], "%d", &port) != 1) {
    printf("Could not parse the arguments");
    exit(-1);
  }

  /* creating socket */
  int sockfd;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  /* setting server address and binding socket */
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET; 
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }


  udp_info clients[MAXCLIENTS];
  memset(clients, 0, MAXCLIENTS*sizeof(udp_info));

  manage_io(sockfd, clients);

  return 0;
}

void manage_io(int sockfd, udp_info *clients){

  int num_clients = 0;
  pthread_t threads[MAXCLIENTS];

  while(1){

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));

    udp_info info;
    info.sockfd = sockfd;
    info.client_addr = client_addr;
    info.len = sizeof(struct sockaddr);
    info.is_addr_valid = 0;

    info.nbytes = recvfrom(info.sockfd, (char *)info.buffer, MAXLINE, MSG_WAITALL,
                            (struct sockaddr *)&info.client_addr, &info.len);

    if (info.nbytes < 0){
      perror("recv error");
    } else {

      info.is_addr_valid = 1;
      info.buffer[info.nbytes] = '\0';

      int i = 0;
      int is_client_registered = 0;
      for(i=0; i<num_clients; i++){
        if(memcmp(&info.client_addr, &clients[i].client_addr, sizeof(info.client_addr)) == 0){

          pthread_mutex_lock(clients[i].lock);
          memcpy(&clients[i], &info, sizeof(udp_info));
          pthread_mutex_unlock(clients[i].lock);

          is_client_registered = 1;

          break;
        }
      }

      printf("Received this message from the Client :\n%s\n", info.buffer);

      if(!is_client_registered && num_clients >= MAXCLIENTS){
        printf("Client rejected: Maximum number of connections have been established\n");
      }

      if(is_client_registered == 1){
        printf("Client already registered. Updating message\n");
      }

      if(!is_client_registered && num_clients<MAXCLIENTS){

        info.lock = &lock[num_clients];
        if (pthread_mutex_init(&lock[num_clients], NULL) != 0)
        {
          printf("\n mutex init failed\n");
          return 1;
        }

        memcpy(&clients[num_clients], &info, sizeof(udp_info));

        pthread_create(&threads[num_clients], NULL, send_messages, &clients[num_clients]);
        num_clients += 1;
        printf("New client accepted\n");
      }

      pthread_mutex_unlock(&lock);
      
    }
  }
}

void *send_messages(udp_info *info)
{

  while (1)
  {

    usleep(DELAY);

    pthread_mutex_lock(info->lock);

    if (sendto(info->sockfd, (const char *)info->buffer, info->nbytes,
               MSG_CONFIRM, (const struct sockaddr *)&info->client_addr, info->len) < 0)
    {

      perror("sendto error");
    }

    pthread_mutex_unlock(info->lock);
  }
  return NULL;
}

/* functions from previous version */

void *echo_forever(int *sockfd_ptr){

  int sockfd = *sockfd_ptr;

  /* receiving the message */
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));

  udp_info info;
  info.sockfd = sockfd;
  info.client_addr = client_addr;
  info.len = sizeof(struct sockaddr);
  info.is_addr_valid = 0;

  pthread_t thread_listen;
  if (pthread_create(&thread_listen, NULL, listen_messages, &info)) {
    fprintf(stderr, "Error creating listening thread\n");
  }

  while(1){
    send_messages(&info);
  }
}

void *listen_messages(udp_info *info){
  while(1){
    info->nbytes = recvfrom(info->sockfd, (char *)info->buffer, MAXLINE, MSG_WAITALL,
                          (struct sockaddr *)&info->client_addr, &info->len);

    if (info->nbytes < 0) {
      perror("recv error");
      continue;
    }

    info->is_addr_valid = 1;
    info->buffer[info->nbytes] = '\0';

    printf("Received this message from the Client :\n%s\n", info->buffer);
  }
  return NULL;
}