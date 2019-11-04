#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "cliente-tcp.h"

#define MAX_SIZE 16000

int get_sum_of_ints_tcp(int sockfd, uint32_t *tab, size_t length, uint32_t *rep)
{
    ssize_t s, sent_bytes = 0, recv_bytes = 0, total_size = (length + 1) * sizeof(uint32_t);
    uint32_t *newtab = malloc(total_size);
    if (newtab == NULL) {
        perror("malloc");
        return -1;
    }
    newtab[0] = htonl(length);
    for (unsigned int i = 0; i < length; i++) {
        newtab[i + 1] = htonl(tab[i]);
    }
    do {
        s = send(sockfd, ((void*)(newtab) + sent_bytes), total_size - sent_bytes, 0);
        if (s == -1) {
            perror("send");
            free(newtab);
            return -2;
        }
        sent_bytes += s;
    } while (sent_bytes != total_size);
    free(newtab);
    // Done with sending, now waiting for an answer.
    uint32_t answer[length];
    do {
        s = recv(sockfd, ((void*)(answer) + recv_bytes), (total_size - recv_bytes), 0);
        if (s == -1) {
            perror("recv");
            return -2;
        }
        recv_bytes += s;
    } while (recv_bytes != sizeof(uint32_t));
    for(int i=0; i < length; i++ )
	rep[i] = ntohl(answer[i]);
    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd; 
    struct sockaddr_in   servaddr;
    int status;
    struct hostent *hostinfo;
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (argc < 3) {
      printf("Uso: ./cliente <servidor> <porta>\n");
      exit(1);
    }

    long int port_number;
    if (sscanf(argv[2], "%ld", &port_number) != 1) {
      printf("Error converting the port number.\n");
      exit(1);
    }
    hostinfo = gethostbyname(argv[1]); 
    memset(&servaddr, 0, sizeof(servaddr)); 
    // Filling server information
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port_number);
    memcpy(&servaddr.sin_addr, hostinfo->h_addr_list[0], hostinfo->h_length);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
      perror("connect");
      close(sockfd);
      return -1;
    }
    unsigned int resultado[4];
    int n = 4;  // tamanho do vetor
    int tablen = n*sizeof(uint32_t);
    uint32_t tab[n];
    for (int i=0; i < n; ++i)
      tab[i] = i+1;

    status = get_sum_of_ints_tcp(sockfd, tab, n, resultado);
    
    if (!status) {
	for(int i=0; i < n; i++){
	   printf("Resultado: %u\n", resultado[i]);
	}
	return 0;
    }  else {
      printf("Erro: %d\n", status);
      return 1;
    }
}

