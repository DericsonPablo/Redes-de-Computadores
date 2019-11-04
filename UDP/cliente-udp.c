#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>

#include "cliente-udp.h"

#define MAX_SIZE 16000

int get_sum_of_ints_udp_sol(int sockfd, uint32_t *tab, size_t length, uint32_t *rep) // parametros descritor do socket
//ponteiro que aponta para o vetor tab, que vem do main, um parametro de tamanho, e outro ponteiro apontando pra rep
{
	// @@client_io_udp@start@@
	/*
	 * We assume that sockfd is an open socket,
	 * already set up to send packets to the server,
	 * and that we only have to send one message to the server,
	 * and receive its answer (one message too).
	 * Because it's UDP.
	 */
	if (length > MAX_SIZE) { //testa para ver se o pacote é suportado para ser enviado via UDP
		fprintf(stderr, "Packet is too large to be sent over UDP");
		return -3;
	}
	ssize_t sent_bytes = 0, recv_bytes = 0; //declara variavel para armazenar bytes enviados e recebidos
	size_t total_bytes = length * sizeof(uint32_t); //seta o total de bytes multiplicando o tamanho por 32 para alocar um
//ponteiro de inteiros
	uint32_t *newtab = malloc(total_bytes); //alocando o ponteiro de inteiros
	if (newtab == NULL) { //testa para erro do malloc
		perror("malloc");
		return -1;
	}
	for (unsigned int i = 0; i < length; i++) {
		newtab[i] = htonl(tab[i]);	
	}
	sent_bytes = send(sockfd, newtab, total_bytes, 0);
	if (sent_bytes != (ssize_t) total_bytes) {
		if (sent_bytes == -1)
			perror("send");
		free(newtab);
		return -2;
	}
	free(newtab);
	uint32_t answer[length];
	recv_bytes = recv(sockfd, answer, sizeof(uint32_t)*length, 0);
	if (recv_bytes != sizeof(uint32_t)*length) {
		if (recv_bytes == -1)
			perror("recv");
		return -2;
	}
	for(int i=0; i < length; i++ )
		rep[i] = ntohl(answer[i]);
	return 0;
	// @@client_io_udp@end@@
}


int main(int argc, char *argv[])
{
    int sockfd; //declara o descritor do socket
    struct sockaddr_in   servaddr; //declara struct para armazenamento do endereco do servidor
    int status; //inteiro indicador de status
    struct hostent *hostinfo; //struct que armazena informações do host
  
    // Criando o descritor do socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { //AF_INET para tipo IPv4, SOCK_DGRAM para conexao do tipo UDP
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (argc < 2) {
      printf("Uso: ./cliente <servidor>\n");
      exit(1);
    }

    
    hostinfo = gethostbyname(argv[1]); 
    memset(&servaddr, 0, sizeof(servaddr)); 
    
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(20000);
    memcpy(&servaddr.sin_addr, hostinfo->h_addr_list[0], hostinfo->h_length);

    // Esse connect() apenas associa ao socket UDP um enderço/porta de destino
    // Não existe conexão na comunicação com UDP
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
      perror("connect");
      close(sockfd);
      return -1;
    }
    uint32_t resultado[4];
    int n = 4, tablen = n*sizeof(uint32_t);
    uint32_t tab[4] = {1,2,3,4};

    status = get_sum_of_ints_udp_sol(sockfd, tab, n, resultado);
    
    if (!status) {
	for(int i=0; i < 4; i++){
     	    printf("Resultado: %d\n", resultado[i]);
    	 } return 0;
    }else {
      printf("Erro: %d\n", status);
      return 1;
    }
}

