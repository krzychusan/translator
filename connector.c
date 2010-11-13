#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

static int sock;
static struct sockaddr_in addr; /* server address */
static struct hostent *hp;

static void
handlerClose()
{
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

int
connector(const char *url, char *request, char *output, int outputSize)
{
    if ((hp = gethostbyname(url)) == NULL) {
        perror("gethostbyname");
        return -2;
    }

    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(80);
    addr.sin_family = AF_INET;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("Creating socket error!\n");
        return sock;
    }
    int on = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("connect");
        return -1;
    }

    int requestSize = strlen(request);

    if (send(sock, request, requestSize, 0) != requestSize) {
        printf("send() sent a different number of bytes than expected");
        free(request);
        return -3;
    }
    free(request);
    
    int done = 0;
    while(recv(sock, output, outputSize - 1, 0) != 0){
        if (done) {
            printf("buffor not enough, scattering not implemented yet ;/");
            return -4;
        }
        done = 1;
    }
    //output[outputSize] = '\0';
    handlerClose();
    return 0;
}



