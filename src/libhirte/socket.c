/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket.h"

int create_tcp_socket(uint16_t port) {
        struct sockaddr_in servaddr;

        int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
        if (fd < 0) {
                int errsv = errno;
                return -errsv;
        }

        int yes = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                int errsv = errno;
                return -errsv;
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);

        if (bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
                int errsv = errno;
                return -errsv;
        }

        if ((listen(fd, SOMAXCONN)) != 0) {
                int errsv = errno;
                return -errsv;
        }

        return fd;
}

int accept_tcp_connection_request(int fd) {
        int nfd = accept4(fd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (nfd < 0) {
                if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                        return 0;
                }

                int errsv = errno;
                return -errsv;
        }
        return nfd;
}

int fd_check_peercred(int fd) {
        int r = 0;
        struct ucred ucred = { .pid = -1, .uid = -1, .gid = -1 };
        socklen_t n = sizeof(struct ucred);

        r = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &n);
        if (r < 0) {
                return -errno;
        }
        if (n != sizeof(struct ucred)) {
                return -EIO;
        }
        // Check if the data is actually useful and not suppressed due to namespacing issues
        if (ucred.pid <= 0) {
                return -ENODATA;
        }
        if (ucred.uid != 0 && ucred.uid != geteuid()) {
                return -EPERM;
        }

        /* Note:
           We don't check UID/GID here as namespace translation works differently in this case.
           Instead of receiving an "invalid" user/group we get the overflow UID/GID.
        */

        return 1;
}
