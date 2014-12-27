#ifndef CLASS_H
#define CLASS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <QMessageBox>
#include <QtDebug>
#include <openssl/ssl.h>
#include <openssl/err.h>

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

SSL_CTX* InitCTX(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = SSLv3_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        qDebug() << "Server certificates:\n";
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        qDebug() << "Subject: " << line << "\n";
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        qDebug() << "Issuer: " << line << "\n";
        free(line);
        X509_free(cert);
    }
    else
        qDebug() << "No certificates.\n";
}

#endif // CLASS_H
