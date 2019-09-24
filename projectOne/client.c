#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/time.h>

#define RSPBUFSIZE 4096 /*buffer size for receiving message */

//www.mit.edu
//www.google.com
//google address is 172.217.12.206
//./client [-options] server_url port_number
//ex: ./client google.com 80
int main(int argc, char **argv)
{
    int sock;/*Socket descriptor */
    int gi; /*Get address info */
    int n; /*counter to parse full url */
    int i; /*counter to print stream */
    char *fullurl; /* agrv[1] or argv[2] depending */
    char *url; /* domain (www.google.com) */
    char *path; /* /maps */
    char response[RSPBUFSIZE]; /*char to print response */
    FILE *stream; /*file pointer for stream*/
    float RTT; /*RTT value*/
    struct timeval first; /*start RTT time */
    struct timeval second; /*end RTT time*/
    struct addrinfo *servInfo; /*Server info store from getaddrinfo()*/

    //check to see if args less than 3 or greater than 4
    if ((argc > 4) || (argc < 3))
    {
        printf("Usage: %s [-options] server_url port_number\n", argv[0]);
        exit(1);
    }
    //no option -p
    if (argc == 3)
    {
        /*parse argv[1] to find domain and path*/
        fullurl = argv[1];/* store url in variable*/
        url = strtok(fullurl, "/"); /*store first part of url as domain*/
        n = (strlen(url) + 1); /*find length of domain */
        path = &fullurl[n - 1]; /*set path to go from the end of domain to the next '/0' */
        if ((gi = getaddrinfo(url, argv[2], 0, &servInfo)) != 0) /*get address info and store, also ensure it is not equal to 0*/
        {
            printf("getaddrinfo() failed\n");
            exit(1);
        }
    }

    //with option -p
    if (argc == 4)
    {
        fullurl = argv[2]; /*store url in variable*/
        url = strtok(fullurl, "/"); /*store first part of url as domain*/
        n = (strlen(url) + 1);/*find length of domain */
        path = &fullurl[n - 1]; /*set path to go from the end of domain to the next '/0' */
        if ((gi = getaddrinfo(url, argv[3], 0, &servInfo)) != 0) //get address info 
        {
            printf("getaddrinfo() failed\n");
            exit(1);
        }
        //check if -p is option
        if(!(strstr(argv[1], "-p")))
        {
            printf("option must be -p, was %s\n", argv[1]);
            exit(1);
        }
    }

    /*creating stream socket  and ensure it is not < 0 */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        printf("socket() failed");
        exit(1);
    }
    /*puts("sock() success"); debugging code */
    
    /*start RTT time*/
    gettimeofday(&first, NULL);
    /* Establish the connection to server */
    if (connect(sock, servInfo->ai_addr, servInfo->ai_addrlen) < 0)
    {
        printf("connect() failed\n");
        exit(1);
    }
    /*end RTT time*/
    gettimeofday(&second, NULL);

    /*open stream and print to it*/
    stream = fdopen(sock, "r+b");
    fprintf(stream, "GET /%s HTTP/1.1\r\n", path);
    fprintf(stream, "Host: %s\r\n", url);
    fprintf(stream, "Content-Type: text/plain\r\n");
    fprintf(stream, "Connection: close\r\n");
    fprintf(stream, "\r\n");

    /*get lines from stream and print*/
    printf("Response\n");
    i = 0;
    while (!i)
    {
        fgets(response, RSPBUFSIZE, stream);
        if ((!strstr(response, "\r\n"))) /*only printing the header not file that was downloaded*/
        {
            i = 1;
        }
        else
        {
            printf("%s", response);
        }
    }

    printf("\n");
    /*only done with -p option */
    if(argc==4)
    {
        float firstEq = ((second.tv_sec - first.tv_sec)*1000);
        float secondEq = ((second.tv_usec - first.tv_usec)/1000);
        RTT = (firstEq + secondEq);
        printf("Round Trip Time (in milliseconds): %f\n", RTT);
        printf("\n");
    }
    close(sock);
    exit(0);
}

    /* old code that has been scraped, kept for reference */
    /* using send() and recv()

    sendString = "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", path, url;

    char *a = "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n";

    int b = strlen(a);
    int c = strlen(path);
    int d = strlen(url);
    int e = (b+c+d);
    printf("%d\n", e);

    sendStringLength = strlen(sendString);

    int sendStringLength2 = strlen(("GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", path, url));

    //printf("%d\n", sendStringLength2);

    printf("GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", path, url);

    //old send: send(sock, sendString, sendStringLength, 0)
    if(send(sock, ("GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", path, url) , e, 0) != e){
        printf("send() failed\n");
        exit(1);
    }
    else{
        printf("send() successful\n");
    }

    */
    /*
    //receive string back from server
    tBytesRecv = 0;
    printf("Received: ");
    while (tBytesRecv < e)
    {
        if((bytesRecv = recv(sock, response, RSPBUFSIZE - 1, 0)) <= 0)
        {
            printf("bytes recv: %d\n", bytesRecv);
            printf("total bytes recv: %d\n", tBytesRecv);
            printf("recv() failed\n");
            exit(1);
        }
        tBytesRecv += bytesRecv;
        response[RSPBUFSIZE] = '\0';
        printf("%s", response);
    }

    */
