#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "servidor-tcp.h"

#define MAX_SIZE 16000

int server_tcp(int sockfd)
{
    struct sockaddr_storage client_addr; //estrutura designada para suportar tanto IPv4 quanto IPv6
    socklen_t client_addrlen; 
    int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addrlen);
    if (clientfd == -1) {
        perror("accept");
        return -2;
    }

    ssize_t r;
    size_t length = 0, recv_bytes = 0;
    do {
        r = recv(clientfd, ((void*)(&length) + recv_bytes), sizeof(uint32_t) - recv_bytes, 0);
        if (r == -1) {
            perror("recv");
            close(clientfd);
            return -2;
        }
        recv_bytes += r;
    } while (recv_bytes != sizeof(uint32_t));
    length = ntohl(length);
    // length now contains the number of 32-bits integers to read.

    size_t bufsize = length * sizeof(uint32_t);
    uint32_t *buf = malloc(bufsize);
    if (buf == NULL) {
        close(clientfd);
        perror("malloc");
        return -1;
    }

    recv_bytes = 0;
    do {
        r = recv(clientfd, ((void*)(buf) + recv_bytes), (bufsize - recv_bytes), 0);
        if (r == -1) {
            perror("recv");
            close(clientfd);
            free(buf);
            return -2;
        }
        recv_bytes += r;
    } while (recv_bytes != bufsize);

 	int nint = recv_bytes/sizeof(uint32_t); //pega a variavel com o tamanho de bytes e divide por 32 para saber o conteúdo
	uint32_t sum[nint];
	for (int i = 0; i < nint; i++) {
		sum[i] = 0;
		for(int j = 0; j <=i; j++){
			sum[i] += ntohl(buf[j]);
		}
		// ntohl(buf[i]); //locallong, funcao que converte um unsigned int hostlong 
					// da ordem de bytes do host para a ordem de bytes da rede.
	//esse for armazenará a soma das conversões que vem do buffer na ordem de bytes do host na variavel sum
	}
    free(buf);
    for(int i =0; i < nint; i++ ){
		sum[i] = htonl(sum[i]); //netlong, funcao que converte um unsigned int hostlong da ordem de bytes do host para a ordem de bytes da rede.
	}

    ssize_t s = 0;
    size_t sent_bytes = 0;
    do {
        s = send(clientfd, ((void*)(sum) + sent_bytes), bufsize - sent_bytes, 0);
        if (s == -1) {
            perror("send");
            close(clientfd);
            return -2;
        }
        sent_bytes += s;
    } while (sent_bytes != bufsize);

    if (close(clientfd) == -1) {
        perror("close");
        return -2;
    }
    return 0;
}

int main(int argc, char *argv[])
{
  struct sockaddr_in servaddr, cliaddr;
  int sockfd;

  if (argc < 2) {
    printf("Uso: ./servidor <porta>\n");
    exit(1);
  }
  
  short int port_number;
  if (sscanf(argv[1], "%hu", &port_number) != 1) {
    printf("Error converting the port number.\n");
    exit(1);
  }
  printf("Usando a porta %hu\n", port_number);
  
  // Creating socket file descriptor 
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
    perror("socket creation failed"); 
    exit(EXIT_FAILURE); 
  } 
      
  memset(&servaddr, 0, sizeof(servaddr)); 
  memset(&cliaddr, 0, sizeof(cliaddr)); 
      
  // Filling server information 
  servaddr.sin_family    = AF_INET; // IPv4 
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY;
  servaddr.sin_port = htons(port_number); 
  
  // Bind the socket with the server address 
  if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
    { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
    }
  listen(sockfd, 5);
  while (1)
    printf("Status: %d\n", server_tcp(sockfd));

  close(sockfd);
}

