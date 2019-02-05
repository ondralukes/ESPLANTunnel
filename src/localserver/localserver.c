#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

char input[20100];
size_t inputlen;
void HTTPReq(char*msg,size_t size,char start);
size_t Decode(char* data,size_t size);
size_t Encode(char* data,size_t size);
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    char buf[1024];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    printf("Checking connection\n");
    HTTPReq("test",4,'C');
    while(1){
      HTTPReq("",0,'D');
      if(strcmp(input,"COK") == 0) break;
    }
    printf("Connection OK\n");
    printf("Entering CTL terminal\n");
    char *tmp = malloc(128);
    while(1){
    while(1){

      printf("CTL>");
      fflush(stdout);
      char *line = NULL;
      size_t size;
      fflush(stdin);
      getline(&tmp, &size, stdin);
      *strchr(tmp,'\n') = '\0';
      if(strncmp(&tmp[0],"connect",7) == 0) break;
      if(strlen(tmp) > 0){
      HTTPReq(&tmp[0],strlen(&tmp[0]),'C');
      while(1){
        HTTPReq("",0,'D');
      if(strlen(input) > 1){
         if(input[0] == 'C')break;
       }
      }
    }
  }
    while(1) {  // main accept() loop
        printf("Waiting for connection...Type anything to switch back.\n");
        struct timeval timeout;
        fd_set fdset;
        FD_ZERO(&fdset);
        FD_SET(sockfd,&fdset);
        FD_SET(STDIN_FILENO,&fdset);
        int maxfd = (sockfd > STDIN_FILENO)?sockfd:STDIN_FILENO;
        char brk = 0;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        select(maxfd+1,&fdset,NULL,NULL,NULL);
        if(FD_ISSET(STDIN_FILENO,&fdset)){
          int c;
           while ((getchar()) != '\n' && c != EOF);
           break;
         }
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            fcntl(new_fd, F_SETFL, O_NONBLOCK);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("Got connection from %s\n", s);
          HTTPReq(tmp,strlen(tmp),'C');
        while(1){
          HTTPReq("",0,'D');
          if(strcmp(input,"CConnected") == 0) break;
        }
        while(1){
          if ((numbytes = recv(new_fd, buf, 1023, 0)) < 1) {
            if(errno!=EAGAIN&&errno!= EWOULDBLOCK || numbytes == 0){
             printf("RECV FAIL\n");
             break;
           }
           buf[0] = '\0';
           HTTPReq(buf,0,'D');
           } else {
             buf[numbytes] = '\0';
             //printf("Received %d bytes:%s\n",numbytes,buf );
             HTTPReq(buf,numbytes,'D');
           }
           //printf("INPUT:%s\n",input);
          if(input[0] == 'D'){
          if(input[0] != '\0'){
            if (send(new_fd, &input[1], inputlen -1, 0) == -1)
                perror("send");
          }
        } else if(input[0] == 'C'){
          if(strcmp(input,"CDisconnected") == 0) break;
        }
          }
        printf("Closed connection\n");
        close(new_fd);
        HTTPReq("disconnect",10,'C');
      while(1){
        HTTPReq("",0,'D');
        if(strcmp(input,"CDisconnected") == 0) break;
      }
    }
  }
    return 0;
}

void HTTPReq(char*msg,size_t size,char start){
  /* first what are we going to send and where are we going to send it? */
    int portno =        80;
    char *host =        "ondralukes.cz";
    char *message_fmt = "POST /wifi/mwr.php?key=********* HTTP/1.0\r\nHost: ondralukes.cz:80\r\nConnection: close\r\nContent-Length:%d\r\nContent-Type: text/plain\r\n\r\n%c";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[20100],response[20100];

    /* fill in the parameters */
    size_t newsize;
    if(start =='D'){
     newsize= Encode(msg,size);
  } else {
    newsize = size;
  }
    sprintf(message,message_fmt,newsize+1,start);
    total = strlen(message) + newsize;
    memcpy(message+strlen(message),msg,newsize);
    if(start == 'C'){
    printf("[TX][CTL] |%s\n",msg);
  } else if(start =='D' && size != 0){
    printf("[TX][DATA]|(%lu bytes decoded, %lu bytes encoded)\n",size,newsize);
  }
    //printf("Request:\n%s\n",message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) perror("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) perror("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        perror("ERROR connecting");

    /* send the request */
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            perror("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0)
            perror("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total)
        perror("ERROR storing complete response from socket");
    /* close the socket */
    close(sockfd);
    size_t rawlen = 0;
        input[0] = '\0';
    if(strchr(response,'^') != NULL){
       rawlen = received-(strchr(response,'^')+1-&response[0]);
    memcpy(input,strchr(response,'^')+1,rawlen);
  }
    inputlen = Decode(&input[0],rawlen);
    /* process response */
    //printf("Response:\n%s\n",response);
    if(input[0] == 'C'){
      input[inputlen] = '\0';
      printf("[RX][CTL] |%s\n",&input[1]);
      if(strcmp(&input[1],"RESET") == 0){
        printf("=========WARNING=========\n");
        printf("Reset has occured, all timeouts has been resetted to their defaults.\n");
        printf("=========================\n");
      }
    } else if(input[0] == 'D'){
      printf("[RX][DATA]|(%lu bytes encoded, %lu bytes decoded)\n",rawlen-2,inputlen-2);
      ////////////////////DATA HEXDUMP
      /*for(size_t i = 0;i<inputlen-2;i++){
        if(i % 8 == 0){
          printf("\n");
        }
        printf("[%02x]",*(&input[1]+i));
      }
      printf("\n");
      printf("\n");*/
    }
}
size_t Decode(char* data,size_t size){
  char * tmp = malloc(size);
  char*inptr = data;
  char*outptr = tmp;
  size_t newsize = 0;
  for(size_t i = 0;i<size;i++){
    if(*inptr == '\\'){
      if(*(inptr+1) == '\\'){
        *outptr = '\\';
      } else if(*(inptr +1) == '0'){
        *outptr = '\0';
      }
      inptr++;
      i++;
    } else {
      *outptr = *inptr;
    }
    outptr++;
    inptr++;
    newsize++;
  }
  memset(data,0,size);
  memcpy(data,tmp,newsize);
  return newsize;
}
size_t Encode(char* data,size_t size){
  char * tmp = malloc(size*2);
  char*inptr = data;
  char*outptr = tmp;
  size_t newsize = 0;
  for(size_t i = 0;i<size;i++){
    if(*inptr == '\\'){
      *outptr = '\\';
      *(outptr+1) = '\\';
      newsize++;
      outptr++;
    } else if(*inptr == '\0'){
      *outptr = '\\';
      *(outptr+1) = '0';
      newsize++;
      outptr++;
    } else {
      *outptr = *inptr;
    }
    outptr++;
    inptr++;
    newsize++;
  }
  memset(data,0,size);
  memcpy(data,tmp,newsize);
  return newsize;
}
