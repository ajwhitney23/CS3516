/*starter file found on http://beej.us/guide/bgnet/examples/server.c 
resource provdied by teacher for project*/

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
#include <signal.h>

#define MAXPENDING 10 // how many pending connections queue will hold
#define SIZE 4096

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
    int servSock, clntSock; // listen on sock_fd, new connection on clntSock
    struct addrinfo hints, *servinfo;
    struct sockaddr_storage clntAddr; // connector's address information
    socklen_t sinSize;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int gi; //for getaddressinfo()
    FILE *file; //file pointer for request from client
    char *clntReq; //pointer for client request
    char *cp; //point that stores client rquests to be put into file
    char *notFound; // 404
    long length; //length of file
    char *incoming; //buffer for incoming
    char *ok; // 200
    char *resp; //response 
    char *respond; //response buffer

    if (argc != 2)
    {
        printf("Usage: ./http_server <port number>\n");
        exit(1);
    }

    //create struct for hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // uses my IP

    //get adress info from command lines and store in servinfo
    if ((gi = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0)
    {
        printf("getaddrinfo: %s\n", gai_strerror(gi));
        return 1;
    }

    //create socket
    if((servSock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))<0)
    {
        perror("server: socket");
        exit(1);
    }

    //bind to socket to server address
    if (bind(servSock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
    {
        close(servSock);
        perror("server: bind");
        exit(1);
    }
    //listen for incoming connections
    if (listen(servSock, MAXPENDING) < 0)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    //infinite loop
    while (1)
    {                              
        sinSize = sizeof clntAddr; //set size to size of the client's address
        //accept the incoming connection
        clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &sinSize);
        if (clntSock < 0)
        {
            perror("accept");
            continue;
        }
        //print the address connection came from
        inet_ntop(clntAddr.ss_family, get_in_addr((struct sockaddr *)&clntAddr),s, sizeof(s));
        printf("server: got connection from %s\n", s);

        if (!fork())
        {
            // this is the child process
            close(servSock); // child doesn't need the listener
            //create a buffer to be used
            incoming = malloc(SIZE * sizeof(char));
            recv(clntSock, incoming, SIZE, 0);
            //parse the incoming message for new lines and spaces
            clntReq = strtok(strtok(incoming, "\n"), " ");
            cp = strtok(NULL, " ");

            //see if requested file is TMDG
            if (!strcmp(cp, "/"))
            {
                cp = "TMDG.html";
            }
            else
            //iterate through the file to find file
            {
                cp++;
            }
            //open cp in file w/ read only permission
            file = fopen(cp, "r");
            //send 404 if not TMDG.html
            if (file == NULL)
            {
                notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
                send(clntSock, notFound, strlen(notFound), 0);
            }
            //go through file to find properties of file
            if (fseek(file, 0, SEEK_END) == 0)
            {
                length = ftell(file);
                fseek(file, 0, SEEK_SET);
                respond = malloc(SIZE * sizeof(char) * (length + 1));
                fread(respond, sizeof(char), length, file);
            }
            else
            {
                fclose(file);
                exit(1);
            }
            fclose(file);
            //create 200 OK resonse
            ok = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            resp = malloc(strlen(ok) + strlen(respond) + 1);
            //copy ok into resp, then combine resp and the respond buffer
            strcpy(resp, ok);
            strcat(resp, respond);
            //send the reponse
            send(clntSock, resp, strlen(resp), 0);
            close(clntSock);
            exit(0);
        }
        close(clntSock); // parent doesn't need this
    }

    return 0;
}