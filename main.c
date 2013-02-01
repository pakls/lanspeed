/*
    Copyright (c) 2013, Anthony Liu <antliu@gmail.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
 */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT (30000)
#define SZ   (512)

void start_server()
{
    WSADATA wsaData;
    struct sockaddr_in addr;
    char buf[SZ];
    int s;
    int d;

    if (WSAStartup(MAKEWORD(1,1), &wsaData) == SOCKET_ERROR) {
        fprintf (stderr, "Error initialising WSA.\n");
        return;
    }

    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s  < 0) {
        fprintf (stderr, "Error %d opening socket.\n", errno);
        return;
    }

    addr.sin_family       = AF_INET;
    addr.sin_addr.s_addr  = inet_addr("0.0.0.0");
    addr.sin_port         = htons(PORT);

    if (bind(s, (SOCKADDR*) &addr, sizeof(SOCKADDR)) < 0) {
        fprintf (stderr, "Error %d binding socket.\n", errno);
        return;
    }

    if (listen(s, 0) < 0) {
        fprintf (stderr, "Error %d listening socket.\n", errno);
        return;
    }

    while (1) {
        fprintf (stderr, "Waiting\n");
        if ((d = accept(s, NULL, 0)) < 0) {
            fprintf (stderr, "Error accepting socket: %s.\n", strerror(errno));
            return;
        }

        fprintf (stderr, "Accepted\n");

        while (send(d, (const char *)&buf[0], SZ, 0) >= 0) {
            ;
        }

        fprintf (stderr, "Closed\n");
    }
    close(s);
}

int start_client(char *ip)
{
    WSADATA wsaData;
    struct sockaddr_in addr;
    char buf[SZ];
    ssize_t len;
    ssize_t total;
    int s;
    DWORD   t1, t2;
    ssize_t p1;

    if (WSAStartup(MAKEWORD(1,1), &wsaData) == SOCKET_ERROR) {
        fprintf (stderr, "Error initialising WSA.\n");
        return 0;
    }

    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s  < 0) {
        fprintf (stderr, "Error %d opening socket.\n", errno);
        return 0;
    }

    addr.sin_family       = AF_INET;
    addr.sin_addr.s_addr  = inet_addr(ip);
    addr.sin_port         = htons(PORT);

    if (addr.sin_addr.s_addr == INADDR_NONE) {
        fprintf (stderr, "Invalid server address.\n");
        return 0;
    }

    if (connect(s, (const struct sockaddr *) &addr, sizeof(struct sockaddr)) < 0)
    {
        fprintf (stderr, "Connect failed: %s.\n", strerror(errno));
        return -1;
    }

    total = 0;

    t1 = 0;

    while ((len = recv(s, buf, SZ, 0)) > 0) {
        total += len;

        if (!t1) {
            t1 = timeGetTime();
            p1 = total;
        } else {
            double speed;
            t2 = timeGetTime();

            if (t2 - t1 < 1000) {
                continue;
            }

            speed = (float)(total - p1) * 8 / ((float)(t2 - t1) / 1000);
            speed = speed / (1024*1024);
            printf("delta %4u, bytes %10d, speed %.3f Mbps\n",
                    (unsigned int)(t2 - t1), total - p1, speed);

            t1 = t2;
            p1 = total;
        }
    }

    close(s);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 2 && argv[1][0] == 's') {
        start_server();
    } else if (argc == 3 && argv[1][0] == 'c' && start_client(argv[2])) {
    } else {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "    run as server: lanspeed s\n");
        fprintf(stderr, "    run as client: lanspeed c <ip addr>\n");
    }
    return 0;
}
