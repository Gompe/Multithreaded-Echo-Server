/**
 * @file fancyclient.c
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

/* multithreading*/
#include <pthread.h>


#define MAXLINE 1024

typedef struct udp_info{
  int sockfd;
  struct sockaddr *servaddr_ptr
} udp_info;

int read_lines(char *buffer, int buffsize);
void test_read_lines();

void *send_message_to_server(int sockfd, const struct sockaddr *servaddr_ptr);
void *read_message_from_server(int sockfd, struct sockaddr *servaddr_ptr);
void *user_input_manager(udp_info *info);

int main(int argc, char *argv[])
{

  /* checking the arguments from the command line */
  if (argc != 3)
  {
    printf("You need to pass arguments IP_ADDRESS and PORT_NUMBER\n");
    exit(-1);
  }

  int port;
  if (sscanf(argv[2], "%d", &port) != 1) {
    printf("Could not parse the arguments");
    exit(-1);
  }
  char *ip_addr = argv[1];

  /* creating socket */
  int sockfd;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  /* server information */
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, ip_addr, &servaddr.sin_addr.s_addr);

  /* data structure for using udp */
  udp_info info;
  info.sockfd = sockfd;
  info.servaddr_ptr = (const struct sockaddr *) &servaddr;

  /* creating thread that receives user input */
  pthread_t thread_user_input;
  pthread_create(&thread_user_input, NULL, user_input_manager, &info);
  
  while(1){
    read_message_from_server(sockfd, (struct sockaddr *)&servaddr);
  }

  close(sockfd);
  return 0;
}

/* command line input handlers */

int read_line(char *buffer, int buffsize){

  if (fgets(buffer, buffsize, stdin) == NULL){
    printf("Input Error\n");
    exit(-1);
  }
  else {
    return strlen(buffer);
  }

  return 0;
}

int read_lines(char *buffer, int buffsize)
{
  /* got my idea from */
  /* https://stackoverflow.com/questions/13592875/reading-multiple-lines-of-input-with-scanf */

  char *curr = buffer;
  int currsize = 0;
  int shift;

  while (1)
  {
    if (fgets(curr, buffsize - currsize, stdin) == NULL)
    {
      printf("Input Error\n");
      exit(-1);
    }
    else
    {
      if (*curr == '\n')
      {
        /* end of input */
        break;
      }
      shift = strlen(curr);

      currsize += shift;
      curr += shift;
    }
  }

  return currsize;
}


void *user_input_manager(udp_info *info){
  int sockfd = info->sockfd;
  const struct sockaddr *servaddr_ptr = info->servaddr_ptr;

  while(1) {
    send_message_to_server(sockfd, servaddr_ptr);
  }
}

void *send_message_to_server(int sockfd, const struct sockaddr *servaddr_ptr){
  char msg[MAXLINE];
  
  // int msglen = read_lines(msg, MAXLINE);
  int msglen = read_line(msg, MAXLINE);

  sendto(sockfd, (const char *)msg, msglen,
         MSG_CONFIRM, servaddr_ptr,
         sizeof(*servaddr_ptr));

  printf("Message sent.\n");
}

void *read_message_from_server(int sockfd, struct sockaddr *servaddr_ptr){
  char buffer[MAXLINE];

  int len = sizeof(struct sockaddr);
  int nbytes_response = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                                servaddr_ptr, &len);

  buffer[nbytes_response] = '\0';
  printf("%s", buffer);
}