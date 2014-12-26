#ifndef CLASS_H
#define CLASS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <QMessageBox>

void errexit(std::string format){
    QMessageBox error;
    error.critical(0, "error", QString::fromStdString(format));
    exit(1);
}

void errwarning(std::string format){
    QMessageBox error;
    error.critical(0, "error", QString::fromStdString(format));
}

int connectsock(const char *host, u_short service){
    struct hostent *phe;
    struct sockaddr_in sin;
    int sd;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(service);

    if ( (phe = gethostbyname(host)) )
            memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        else if ((sin.sin_addr.s_addr=inet_addr(host)) == INADDR_NONE)
            errexit("can’t get host entry");

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errexit("Failed to allocate to server.");

    if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Faild to connect to server");

    return sd;
}

int passivesock(u_short service, int qlen){
    struct sockaddr_in sin;
    int s;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(service);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        errexit("socket");

    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("bind");

    if (listen(s, qlen) < 0)
        errexit("listen");

    return s;
}

#endif // CLASS_H
