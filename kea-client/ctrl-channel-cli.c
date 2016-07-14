// Copyright (C) 2015 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main(int argc, const char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s socket_path\n", argv[0]);
        return (1);
    }
    // Create UNIX stream socket.
    int socket_fd;
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create UNIX stream");
        return (1);
    }
    // Specify the address to connect to (unix path)
    struct sockaddr_un srv_addr;
    memset(&srv_addr, 0, sizeof(struct sockaddr_un));
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, argv[1]);
    socklen_t len = sizeof(srv_addr);
    // Try to connect.
    if (connect(socket_fd, (struct sockaddr*) &srv_addr, len) == -1) {
        perror("Failed to connect");
        return (1);
    }
    // Send a command to list all available commands.
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    sprintf(buf, "{ \"command\": \"list-commands\" }");
    int bytes_sent = send(socket_fd, buf, strlen(buf), 0);
    printf("%d bytes sent\n", bytes_sent);

    // Receive a response (should be JSON formatted list of commands)
    memset(buf, 0, sizeof(buf));
    int bytes_rcvd = recv(socket_fd, buf, sizeof(buf), 0);
    printf("%d bytes received: [%s]\n", bytes_rcvd, buf);
    // Close the socket
    close(socket_fd);
    return 0;
}
