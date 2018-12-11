/*******************************************************************************
 * @author  Joseph Kamel 
 * @email   josephekamel@gmail.com
 * @date    28/11/2018
 * @version 2.0
 *
 * SCA (Secure Cooperative Autonomous systems)
 * Copyright (c) 2013, 2018 Institut de Recherche Technologique SystemX
 * All rights reserved.
 *******************************************************************************/

#include <veins/modules/application/f2mdVeinsApp/mdSupport/HTTPRequest.h>

using namespace std;
HTTPRequest::HTTPRequest(int portno, std::string host){
    this->portInt = portno;
    this->hostStr = host;
}

std::string HTTPRequest::formaliseString(std::string arg){
    size_t index = 0;
    while (true) {
         /* Locate the substring to replace. */
         index = arg.find(" ", index);
         if (index == std::string::npos) break;

         /* Make the replacement. */
         arg.replace(index, 1, "%");
         arg.insert(index+1, "20");
         /* Advance index forward so the next iteration doesn't pick it up as well. */
         index += 1;
    }

    index = 0;
    while (true) {
         /* Locate the substring to replace. */
         index = arg.find("\t", index);
         if (index == std::string::npos) break;

         /* Make the replacement. */
         arg.replace(index, 1, "%");
         arg.insert(index+1, "09");
         /* Advance index forward so the next iteration doesn't pick it up as well. */
         index += 1;
    }

    index = 0;
    while (true) {
         /* Locate the substring to replace. */
         index = arg.find("\n", index);
         if (index == std::string::npos) break;

         /* Make the replacement. */
         arg.replace(index, 1, "%");
         arg.insert(index+1, "0A");
         /* Advance index forward so the next iteration doesn't pick it up as well. */
         index += 1;
    }

    return arg;
}

char message[16384];
char response[16384];

std::string HTTPRequest::Request(std::string requestArg) {
    /* first what are we going to send and where are we going to send it? */
    int portno = portInt;
    const char *host = hostStr.c_str();
    const char *message_fmt = "POST %s HTTP/1.0\r\n\r\n";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;

    response[0] = '0';

    std::string arg = formaliseString(requestArg);

    /* fill in the parameters */
    sprintf(message, message_fmt, arg.c_str());
    //printf("Request:\n%s\n", message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        printf("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL)
        printf("ERROR, no such host");


    /* fill in the structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR connecting\n");
        return "ERROR connecting";
    }

    /* send the request */
    total = strlen(message);


    sent = 0;
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0)
            printf("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);



    /* receive the response */
    memset(response, 0, sizeof(response));
    total = sizeof(response) - 1;


    received = 0;
    do {
        bytes = read(sockfd, response + received, total - received);
        if (bytes < 0)
            printf("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received += bytes;
    } while (received < total);

    if (received == total)
        printf("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    /* process response */
    //printf("Message:\n%s\n", message);
    //printf("Response:\n%s\n", response);

    return response;

}

