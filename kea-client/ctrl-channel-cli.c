// Copyright (C) 2015-2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// Attempts to load a file into buffer and wrap it with
/// config-set JSON syntax.
/// @return 0 on failure, 1 on success
int loadConfig(char* buf, size_t buflen, const char* filename) {

    const char* COMMAND_PREFIX =
        "{\n"
        "    \"command\": \"config-set\",\n"
        "    \"arguments\": \n";
    const char* COMMAND_SUFFIX =
        "    \n"
        "}\n";

    FILE* f = NULL;
    f = fopen(filename, "r");
    if (!f) {
        printf("Failed to open file %s\n", filename);
        return (0);
    }

    sprintf(buf, "%s", COMMAND_PREFIX);
    size_t offset = strlen(buf);

    size_t len = fread(buf + offset, sizeof(char), buflen - offset, f);

    fclose(f);

    sprintf(buf + strlen(buf), "%s", COMMAND_SUFFIX);

    return (1);
};

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s socket_path [config-file]\n", argv[0]);
        printf("socket_path is mandatory\n");
        printf("If optional config-file is specified, its content is sent to Kea\n");
        printf("If not, list-commands command is sent that will ask Kea to list\n");
        printf("all supported commands.\n");
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
    // Prepare input buffer.
    char buf[10240];
    memset(buf, 0, sizeof(buf));

    if (argc == 3) {
        if (!loadConfig(buf, sizeof(buf), argv[2])) {
            printf("Failed to load specified config file: %s", argv[2]);
            return (1);
        }
    } else {
        // Send a command to list all available commands.
        sprintf(buf, "{ \"command\": \"list-commands\" }");
    }

    printf("Buffer to be sent: %s\n", buf);

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
