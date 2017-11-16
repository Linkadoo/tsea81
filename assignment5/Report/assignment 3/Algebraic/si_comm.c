#include "si_comm.h"
#define PTHREADS
#ifndef PTHREADS
#include "console.h"
#endif 

/* NOTE: this module is currently not adapted for Simple_OS on ARM */ 

#if defined BUILD_X86_HOST || defined BUILD_X86_64_HOST || defined PTHREADS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 

#ifndef BUILD_X86_WIN_HOST

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#endif

#ifdef BUILD_X86_WIN_HOST

#include<winsock2.h>
// #include <Ws2tcpip.h>

typedef int socklen_t; 

#endif

#endif

#define SI_COMM_ERROR (-1)
#define SI_COMM_EMPTY 1

#if defined BUILD_X86_HOST || defined BUILD_X86_64_HOST || defined PTHREADS

#ifdef BUILD_X86_WIN_HOST
static SOCKET newsockfd = -1; 
static SOCKET sockfd = -1; 
#else
static int newsockfd = -1; 
static int sockfd = -1; 
#endif

#endif

static int Connection_Ok; 

static void error(const char *msg)
{
#ifdef BUILD_ARM_BB
#else
    perror(msg);
    exit(1);
#endif
}

void si_comm_open(void)
{
#ifdef BUILD_ARM_BB
    Connection_Ok = 1; 
#else
    struct sockaddr_in serv_addr; 

#ifndef BUILD_X86_WIN_HOST
    struct sockaddr_in cli_addr;
    socklen_t clilen;
#endif

    int portno; 
    int optval = 1; 

#ifdef BUILD_X86_WIN_HOST

    WSADATA wsaData;
    int iResult = 0;

    Connection_Ok = 0; 

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        error("Error at WSAStartup()");
    }

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    { 
       error("ERROR opening socket");
    }
#else
    Connection_Ok = 0; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    { 
       error("ERROR opening socket");
    }
#endif

#ifdef BUILD_X86_WIN_HOST
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(optval)); 
#else
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); 
#endif

    memset(&serv_addr, 0, sizeof(serv_addr)); 
    // bzero((char *) &serv_addr, sizeof(serv_addr));

#ifndef BUILD_X86_WIN_HOST
    memset(&cli_addr, 0, sizeof(cli_addr)); 
    clilen = sizeof(cli_addr); 
#endif

    portno = 2000; 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); 

    // printf("calling bind\n"); 
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) 
             error("ERROR on binding");

    // printf("back from bind\n"); 

    // printf("calling listen\n"); 
    listen(sockfd,5);
    // printf("back from listen\n"); 

    printf("waiting for socket connection ...\n");
#ifdef BUILD_X86_WIN_HOST
    newsockfd = accept(sockfd, NULL, NULL); 
    if (newsockfd == INVALID_SOCKET)
    {
         error("ERROR on accept");
    }
#else
    newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr,
                &clilen);
    if (newsockfd < 0)
    {
         error("ERROR on accept");
    }
#endif
    printf("connection established\n");
    Connection_Ok = 1; 
#endif
}

int si_comm_read(char message_data[], int message_data_size)
{
#ifdef BUILD_ARM_BB
    int read_status; 
    char c; 
    int pos;
    int n_tries; 
    int max_tries = 100000;  
    int end_found; 
    int stop_reading; 
    int char_found; 

    read_status = console_get_char(&c); 
    // console_put_char(c); 

    if (read_status == -1)
    {
        return SI_COMM_ERROR; 
    }
    if ((int) c < 32)
    {
        return SI_COMM_ERROR; 
    }

    // console_put_char((int) c + 1); 

    // there seems to be some writable characters for us

    // store first character
    message_data[0] = c; 

    // look for the rest
    end_found = 0; 
    stop_reading = 0; 
    pos = 1; 
    while (!end_found && !stop_reading)
    {
        // console_put_string("after a while\n"); 
        n_tries = 0; 
	char_found = 0; 
        while (n_tries < max_tries && !char_found)
        {
            read_status = console_get_char(&c); 
            char_found = read_status != 1 && (int) c >= 32; 
            n_tries++; 
        } 
        // console_put_string("after DO while\n"); 
        // console_put_char(c); 
        if (char_found)
	{
            // console_put_char(c); 
            if (c == '#')
	    {
                end_found = 1; 
                stop_reading = 1; 
	    }
            else
	    {
                message_data[pos] = c; 
                pos++; 
	    }
	}
	else
	{
            console_put_char(c); 
            stop_reading = 1; 
	}
    }
    if (end_found)
    {
        // console_put_string("end found\n"); 
        message_data[pos] = '\0'; 
        return SI_COMM_OK; 
    }
    else
    {
        // console_put_string("end NOT found\n"); 
        return SI_COMM_ERROR; 
    }
#else
    fd_set read_fds; 
    struct timeval waitd; 
    int stat; 
    int n; 

    int return_value; 

    waitd.tv_sec = 0;  
    waitd.tv_usec = 0; 

    FD_ZERO(&read_fds);
    FD_SET(newsockfd, &read_fds); 
    
    stat = select(newsockfd + 1, &read_fds, NULL, NULL, &waitd);

    if (stat < 0)
    {    
        printf("SELECT ERROR\n");
        return SI_COMM_ERROR; 
    }

    if (!(FD_ISSET(newsockfd, &read_fds)))
    {
        /* nothing to read, but otherwise OK */ 
        return SI_COMM_EMPTY; 
    }

    // printf("there seems to be data to be read\n");

    /* read the data */ 
#ifdef BUILD_X86_WIN_HOST
    n = recv(newsockfd,message_data,255,0);
#else
    n = read(newsockfd,message_data,255);
#endif
    if (n < 0) 
    {
        printf("ERROR reading from socket"); 
        return SI_COMM_ERROR; 
    }

    /* now we have data, let's check how many */ 
    if (n > message_data_size-1)
    {
        /* too many */
        message_data[message_data_size-1] = '\0'; 
        return_value = SI_COMM_ERROR; 
    }
    else
    {
        /* everything is ok - we have some data! */ 
        message_data[n-1] = '\0'; 
        return_value = SI_COMM_OK; 
    }           
    // printf("Here is the message: %sSTOP\n", message_data);

    return return_value; 
#endif
}

int si_comm_write(const char message_data[])
{
#ifdef BUILD_ARM_BB
    if (Connection_Ok)
    {
        console_put_string(message_data); 
    }        
    return SI_COMM_OK; 
#else
    int n; 
#ifdef BUILD_X86_WIN_HOST
    int wsa_error; 
#endif

    // printf("si_comm_write: %s\n", message_data); 
    if (Connection_Ok)
    {
#ifdef BUILD_X86_WIN_HOST
        n = send(newsockfd,message_data, strlen(message_data), 0); 
        if (n == SOCKET_ERROR)
        {
            wsa_error = WSAGetLastError(); 
            printf("ERROR writing to socket - wsa_error: %d\n", wsa_error); 
            error("ERROR writing to socket");
        }
#else
        n = write(newsockfd,message_data, strlen(message_data)); 
        if (n < 0) error("ERROR writing to socket");
#endif
    }
    return SI_COMM_OK; 
#endif
}

void si_comm_close(void)
{
#ifdef BUILD_ARM_BB
#else
#ifdef BUILD_X86_WIN_HOST
    closesocket(sockfd); 
    closesocket(newsockfd); 
#else
    close(sockfd); 
    close(newsockfd); 
#endif
#endif
}
