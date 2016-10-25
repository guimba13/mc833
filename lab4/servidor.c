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
#include <pthread.h>
#include <string.h>

#define LISTENQ 10
#define MAXDATASIZE 100
void* connection_handler(void* socket_desc);

int main (int argc, char **argv) {
  int    listenfd, connfd, *new_sock;
  struct sockaddr_in servaddr,_self, _loopId;
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
    FILE *f;
    f = fopen("connection.log", "a+"); // a+ (create + append) option will allow appending which is useful in a log file
    if (f == NULL) { 
      puts("Something went wrong writing log!");
    }else{
      time_t rawtime;
      struct tm * timeinfo;

      time ( &rawtime );
      timeinfo = localtime ( &rawtime );
    
      fprintf(f, "%sCONNECTION - IP : %s | PORT : %d\n",asctime (timeinfo), str, ntohs(_self.sin_port));  
    }
    fclose(f);
    
    
    //cria uma thread para cada conexão
    pthread_t thread;
    new_sock = malloc(1);
    *new_sock = connfd;
    if (pthread_create(&thread, NULL, connection_handler, (void*) new_sock) < 0) {
      perror("Could not create thread");
      exit(1);
    }
  }
  return(0);
}

//handler utilizado para a thread
void *connection_handler(void* socket_desc) {
  int sock = *(int*)socket_desc, read_size;
  char message[2000];
  struct sockaddr_in _loopId;
  char   str[INET_ADDRSTRLEN];
  char   buf[MAXDATASIZE];

  char buffer[1000];
  char *response = NULL;
  char *temp = NULL;
  unsigned int size = 1;  // start with size of 1 to make room for null terminator
  unsigned int strlength;

  //lê constantemente do socket
  while( (read_size = recv(sock , message , 2000 , 0)) > 0 )
  {

    socklen_t lenS = sizeof(_loopId);

    if (getpeername(sock, (struct sockaddr *)&_loopId, &lenS) == -1)
      perror("getsockname");
    else{
      inet_ntop(AF_INET, &(_loopId.sin_addr), str, INET_ADDRSTRLEN);
      printf("IP: %s | PORT: %d | %s\n", str, ntohs(_loopId.sin_port), message);
    }

    FILE *f;
    if (NULL == (f = popen(message, "r"))) {
      perror("popen");
      exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), f) != NULL) {
      strlength = strlen(buffer);
      temp = realloc(response, size + strlength);  // allocate room for the buffer that gets appended
      if (temp == NULL) {
        // allocation error
      } else {
        response = temp;
      }
      strcpy(response + size - 1, buffer);     // append buffer to str
      size += strlength; 
    }
    pclose(f);

    //Envia a mensagem de volta ao cliente
    write(sock , response , strlen(response));
  }

  //realiza verificações
  if(read_size == 0) {
    socklen_t lenS = sizeof(_loopId);
    if (getpeername(sock, (struct sockaddr *)&_loopId, &lenS) == -1)
      perror("getsockname");
    else{
      inet_ntop(AF_INET, &(_loopId.sin_addr), str, INET_ADDRSTRLEN);
      printf("Client disconnected - IP: %s | PORT: %d | ", str, ntohs(_loopId.sin_port));
    }

    FILE *f;
    f = fopen("connection.log", "a+"); // a+ (create + append) option will allow appending which is useful in a log file
    if (f == NULL) { 
      puts("Something went wrong writing log!");
    }else{
      time_t rawtime;
      struct tm * timeinfo;

      time ( &rawtime );
      timeinfo = localtime ( &rawtime );
      
      fprintf(f, "%sDESCONECTION - IP : %s | PORT : %d\n",asctime (timeinfo), str, ntohs(_loopId.sin_port));  
    }
    fclose(f);
  
    fflush(stdout);
  }
  else if(read_size == -1)
  {
    perror("recv failed");
  }

  // Fecha a conexao com o servidor
  free(socket_desc);

  return 0;
}
