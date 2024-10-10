#include <stdio.h>
#include <stdlib.h>
#include <libssh/libssh.h>
#include <string.h>

void execute_command(ssh_session session, const char *command) {
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;

    // Open a channel
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating channel\n");
        return;
    }

    // Open the channel for execution
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening channel: %s\n", ssh_get_error(channel));
        ssh_channel_free(channel);
        return;
    }

    // Request a PTY
    rc = ssh_channel_request_pty(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error requesting PTY: %s\n", ssh_get_error(channel));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return;
    }

    // Request a shell
    rc = ssh_channel_request_shell(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error requesting shell: %s\n", ssh_get_error(channel));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return;
    }

    // Send the command to the shell
    ssh_channel_write(channel, command, strlen(command));
    ssh_channel_write(channel, "\n", 1);  // Simulate pressing Enter

    // Read the output
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[nbytes] = '\0'; // Null-terminate the buffer
        printf("%s", buffer);  // Print the output
    }

    // Check for errors
    if (nbytes < 0) {
        fprintf(stderr, "Error reading from channel: %s\n", ssh_get_error(channel));
    }

    // Cleanup
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
}

int main() {
    ssh_session my_ssh_session;
    const char* command = "cd /;lsd";  // Command to execute
    const char* host = "192.168.100.6";
    const char* username = "yulai";
    int rc;

    // Initialize the SSH session
    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        fprintf(stderr, "Error creating SSH session\n");
        return EXIT_FAILURE;
    }

    // Set server and user details
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, host);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

    // Connect to the server
    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to host: %s\n", ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        return EXIT_FAILURE;
    }

    // Authenticate (passwordless authentication can be used with keys)
    rc = ssh_userauth_publickey_auto(my_ssh_session, NULL, NULL);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return EXIT_FAILURE;
    }

    // Execute a command
    execute_command(my_ssh_session, command);

    // Cleanup
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return EXIT_SUCCESS;
}
