#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include "servidor-udp.h"

#define MAX_SIZE 16000

int server_udp(int sockfd) //funcao que cria o servidor com parametro descritor do socket
{
	// @@server_io_udp@start@@
	/*
	 * Assume that sockfd is an open socket
	 * which is bound to the server's address,
	 * and (kinda) listening to messages,
	 * so that it can receive messages from clients.
	 * As such, all we have to do is just recvfrom the requests.
	 */
	struct sockaddr_storage client_addr; //estrutura designada para suportar tanto IPv4 quanto IPv6
	socklen_t client_addrlen = sizeof(client_addr);  //seta o tamanho do endereço do cliente de acordo com a variavel client_addr
	size_t bufsize = MAX_SIZE * sizeof(uint32_t); //seta o tamanho do buffer
	uint32_t *buf = malloc(bufsize); //aloca espaço na memória de acordo com o tamanho do buffer
	if (buf == NULL) { //caso o tamanho for nulo retornará erro na alocação do malloc
		perror("malloc");
		return -1;
	}
	ssize_t r = recvfrom(sockfd, buf, bufsize, 0, (struct sockaddr*)&client_addr, &client_addrlen);
 //funcao recvfrom que é usada para receber dados de um socket, permite o recebimento do endereço e número de porta do remetente
	if (r == -1) {
		perror("recvfrom");
		free(buf);
		return -2;
	}

	char end_cliente[INET6_ADDRSTRLEN]; //char com o endereco do cliente
	int err=getnameinfo((struct sockaddr*)&client_addr,client_addrlen,end_cliente,sizeof(end_cliente),0,0,NI_NUMERICHOST);
//procura as informações do nome do host e do serviço para uma determinada struct sockaddr
	if (err!=0) {
	  printf("Falha ao converter endereço (código=%d)\n",err);
	} else
	  printf("Endereço do cliente: %s\n", end_cliente);
	
	size_t recv_bytes = r; //pega o tamanho em numero de bytes do retorno da funcao recvfrom
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
	free(buf); //libera o buffer
	for(int i =0; i < nint; i++ ){
		sum[i] = htonl(sum[i]); //netlong, funcao que converte um unsigned int hostlong da ordem de bytes do host para a ordem de bytes da rede.
	}
	ssize_t s = sendto(sockfd, (void*)sum, sizeof(uint32_t)*nint, 0, (struct sockaddr*)&client_addr, client_addrlen); 
//funcao sendto que é usada para comunicação não orientada a conexão, ou seja, do tipo UDP
//possui os parametro sockfd, descritor do socket, sum, contendo os dados a serem enviados, sizeof, tamanho do buffer, e os ultimos
//parametros contendo o endereço de destino e o tamanho da estrutura que o contém
	if (s != sizeof(uint32_t)*nint) {
		perror("sendto");
		return -2;
	}
	return 0;
	// @@server_io_udp@end@@
}

int main()
{
  struct sockaddr_in servaddr, cliaddr;
  int sockfd; //declara o descritor do socket

  // Criando o descritor do socket
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { // AF_INET para IPv4, SOCK_DGRAM para conexao UDP
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr)); 
//pega uma posicao de memoria e zera para posteriormente aramazenar o
// o endereco do servidor
  memset(&cliaddr, 0, sizeof(cliaddr));
//pega uma posicao da memoria e zera para posteriormente armazenar o 
// endereco do cliente

  // Preenchendo com as informacoes do servidor
  servaddr.sin_family    = AF_INET; //tipo IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY; 
  servaddr.sin_port = htons(20000); //netshort, define a porta
  
  // Faz a Bind do socket com o endereco do servidor 
  if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
    }
  while (1)
    server_udp(sockfd);
  
}
