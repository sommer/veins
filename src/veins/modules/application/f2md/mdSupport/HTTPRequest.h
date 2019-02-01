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

#ifndef __VEINS_HTTPRequest_H_
#define __VEINS_HTTPRequest_H_

#include <tuple>
#include <omnetpp.h>

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <boost/algorithm/string.hpp>

class HTTPRequest {
private:
    int portInt = 80;
    std::string hostStr= "localhost";

public:
    HTTPRequest(int portno, std::string host);
    std::string Request(std::string arg);
    std::string formaliseString(std::string requestArg);

};

#endif
