//
//  IMPORTANT: THIS VERSION IS ABANDONED, USE “clientguiqt” INSTEAD.
//

#include <iostream>
#include <string>
#include <cstring>
#include <string.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <regex.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sstream>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#define BLEN 1200

bool connected = false;

//using namespace std;

int errexit(std::string format){
    std::cout << format << std::endl;
    return 0;
}

int connectsock(const char *host, u_short service, const char *transport){
    struct hostent *phe;
    struct protoent *ppe;
    struct sockaddr_in sin; //an Internet endpoint address
    
    int sd; //socket descriptor
    memset(&sin, 0, sizeof(sin)); //clean up
    sin.sin_family = AF_INET; //address family: Internet
    sin.sin_port = htons(service);
    
    // Map host name to IP address, allowing for dotted decimal
    if ( (phe = gethostbyname(host)) )
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr=inet_addr(host)) == INADDR_NONE)
        errexit("can’t get host entry");
    
    // Map transport protocol name to protocol number
    if ( (ppe = getprotobyname(transport)) == 0)
        errexit("can’t get protocol entry");
    
    //allocate socket
    if ((sd = socket(PF_INET, SOCK_STREAM, ppe->p_proto)) < 0)
        errexit("Failed to allocate to server.");
    
    //connect the socket
    if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Faild to connect to server");
    
    return sd;
}

int passivesock(u_short service, int qlen){
    struct protoent *ppe; // pointer to protocol information entry
    struct sockaddr_in sin; //an Internet endpoint address
    int s; //socket descriptor
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(service);
    
    if ( (ppe = getprotobyname("TCP")) == 0)
        return errexit("getprotobyname");
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return errexit("socket");
    
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        return errexit("bind");
    
    if (listen(s, qlen) < 0)
        return errexit("listen");
    
    return s;
}

void* recving(void *data){
    int sdc = *(int*)data;
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(buf, 0, BLEN);
    
    while (true){
        if (recv(sdc, bptr, buflen, 0) > 0){
            std::cout << "<< " << buf;
            memset(buf, 0, BLEN);
        }
    }
    return 0;
}

void* waiting(void *data){
    int sd = *((int*)data);
    int sdc;
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    
    while (true){
        if ((sdc = ::accept(sd, (struct sockaddr *)&clientAddr, &addrlen)) > 0){
            //if(fcntl(sdc, F_SETFL, fcntl(sdc, F_GETFL) | O_NONBLOCK) < 0) errexit("fctnl");
            connected = true;
            std::cout << "Connected from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
            
            pthread_t recvthread;
            pthread_create(&recvthread, NULL, recving, &sdc);
            
            std::string request = "";
            while (true){
                while (getline(std::cin, request)){
                    if (request != "") break;
                };
                if (request == "#Exit") {
                    std::cout << "Connection End" << std::endl;
                    connected = false;
                    break;
                }
                request = request + "\n";
                while (send(sdc, request.c_str(), strlen(request.c_str()), 0) > 0) break;
            }
        }
    }
    return 0;
}

int main(int argc, const char * argv[]) {
    char req[BLEN];
    char buf[BLEN];
    char *bptr = buf;
    ssize_t buflen = sizeof(buf);
    memset(req, 0, BLEN);
    memset(buf, 0, BLEN);
    int sd, sds, sdc;
    std::string host = "localhost";
    u_short portno = 6900;
    if (argc == 2) portno = strtol(argv[1], NULL, 10);
    if (argc == 3){
        host = argv[1];
        portno = strtol(argv[2], NULL, 10);
    }
    sd = connectsock(host.c_str(), portno, "TCP"); //140.112.107.39:5900
    portno = 0;
    
    std::string name;
    std::cout << "Register name: ";
    std::cin >> name;
    
    std::string reqs = "REGISTER#" + name;
    if (send(sd, reqs.c_str(), strlen(reqs.c_str()), 0) < 0)
        errexit("Failed to send the request");
    if (recv(sd, bptr, buflen, 0) < 0)
        errexit("Failed to receive the response.");
    std::cout << buf;
    
    regex_t logre;
    std::string log = "#(.*)#(.*)";
    memset(buf, 0, BLEN);
    if (send(sd, "List", 4, 0) < 0)
        errexit("Failed to send the request");
    if (recv(sd, bptr, buflen, 0) < 0)
        errexit("Failed to receive the response.");
    regcomp(&logre, (name + log).c_str(), REG_EXTENDED);
    regmatch_t matches[3];
    if (regexec(&logre, buf, 3, matches, 0) == 0){
        buf[matches[1].rm_eo] = 0;
    }
    portno = (u_short) strtoul(buf+ matches[2].rm_so, NULL, 0);
    
    std::stringstream ss;
    ss << portno;
    memset(buf, 0, BLEN);
    std::string login = name + "#" + ss.str() + "\n";
    if (send(sd, login.c_str(), strlen(login.c_str()), 0) < 0)
        errexit("Failed to send the request");
    if (recv(sd, bptr, buflen, 0) < 0)
        errexit("Failed to receive the response.");
    std::cout << buf;
    memset(buf, 0, BLEN);
    
    sds = passivesock(portno+1, SOMAXCONN);
    pthread_t serverThread;
    if (pthread_create(&serverThread, NULL, waiting, &sds) < 0) errexit("thread");
    
    std::string format;
    regex_t talkwithserver;
    while(std::cin >> format && connected == false){
        regcomp(&talkwithserver, "(.*)", REG_EXTENDED);
        regmatch_t matches[1];
        if (regexec(&talkwithserver, format.c_str(), 1, matches, 0) == 0){
            if (format == "List"){
                if (send(sd, "List", 4, 0) < 0)
                    errexit("Failed to send the request");
                memset(buf, 0, BLEN);
                if (recv(sd, bptr, buflen, 0) < 0)
                    errexit("Failed to receive the response.");
                std::cout << buf;
            }
            else if (format == "Exit"){
                if (send(sd, "Exit", 4, 0) < 0)
                    errexit("Failed to send the request");
                memset(buf, 0, BLEN);
                if (recv(sd, bptr, buflen, 0) < 0)
                    errexit("Failed to receive the response.");
                std::cout << buf;
                break;
            }
            else{
                if (send(sd, "List", 4, 0) < 0)
                    errexit("Failed to send the request");
                memset(buf, 0, BLEN);
                if (recv(sd, bptr, buflen, 0) < 0)
                    errexit("Failed to receive the response.");
                regex_t logre;
                regcomp(&logre, "(.*)#(.*)", REG_EXTENDED);
                regmatch_t matches[2];
                char *ip, *port;
                strcpy(buf, format.c_str());
                if (regexec(&logre, buf, 3, matches, 0) == 0){
                    buf[matches[1].rm_eo] = 0;
                    ip = buf + matches[1].rm_so;
                    buf[matches[2].rm_eo] = 0;
                    port = buf + matches[2].rm_so;
                }
                if ((sdc = connectsock("localhost", ((u_short) strtoul(port, NULL, 0)) + 1, "TCP"))){
                    //if(fcntl(sdc, F_SETFL, fcntl(sdc, F_GETFL) | O_NONBLOCK) < 0) errexit("fctnl");
                    std::cout << "Connected to " << ip << std::endl;
                    
                    pthread_t recvthread;
                    pthread_create(&recvthread, NULL, recving, &sdc);
                    
                    std::string request = "";
                    while (true){
                        while (getline(std::cin, request)){
                            if (request != "") break;
                        };
                        if (request == "#Exit") {
                            std::cout << "Connection End" << std::endl;
                            break;
                        }
                        request = request + "\n";
                        while (send(sdc, request.c_str(), strlen(request.c_str()), 0) > 0) break;
                    }
                }
            }
        }
    }
    
    if (shutdown(sd, SHUT_RDWR) < 0)
        return errexit("shutdown");
    return 0;
}
