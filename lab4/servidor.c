#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>


#define LISTENQ 10
#define MAXDATASIZE 100

int main (int argc, char **argv) {
   int    listenfd, connfd;
   struct sockaddr_in servaddr,_self;
   char   buf[MAXDATASIZE];
   char   str[INET_ADDRSTRLEN];
   time_t ticks;

// Começa a escutar o socket para a conexao
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
   }

	// Inicializa a variavel de endereco
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(13000);

	// Da bind na porta e no socket e trata o erro
   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }

	// Escuta o socket da conexao e trata o erro
   if (listen(listenfd, LISTENQ) == -1) {
      perror("listen");
      exit(1);
   }

	// Mantem o servidor rodando infinitamente
   for ( ; ; ) {
	   // Aceita a conexao de um cliente e trata o erro
      socklen_t len = sizeof(_self);
      if ((connfd = accept(listenfd, (struct sockaddr *) &_self, &len)) == -1 ) {
         perror("accept");
         exit(1);
      }
      inet_ntop(AF_INET, &(_self.sin_addr), str, INET_ADDRSTRLEN);
      //chama a função getpeername para pegar o endereço ip e porta do socket remoto
      printf("IP SOCKET REMOTO: %s\nPORT SOCKET REMOTO: %d\n", str, ntohs(_self.sin_port));


	  // Pega a hora atual e envia para o cliente
      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
      write(connfd, buf, strlen(buf));

	  // Fecha a conexao com o servidor
      close(connfd);
   }
   return(0);
}