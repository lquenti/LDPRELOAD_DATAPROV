#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

bool data_received = false;
char *received_data = NULL;

#define SOCKET_PATH "/tmp/premain_injector_sock"
#define BUF_SIZE 1024

/**
 * @brief The entry point for the background listener thread.
 * * Sets up a Unix Domain Socket, listens for a single connection,
 * reads all data until EOF, stores it globally, and sets the flag.
 * * @param arg Unused argument (NULL).
 * @return void* Always returns NULL.
 */
void *socket_listener(void *arg) {
  int listen_fd, client_fd;
  struct sockaddr_un addr;

  // 1. Create socket file descriptor (AF_UNIX for local IPC)
  listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    perror("Error creating listener socket");
    return NULL;
  }

  // Ensure the socket path is cleaned up before binding
  unlink(SOCKET_PATH); 

  // 2. Prepare the socket address structure
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  // 3. Bind the socket to the path
  if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
    perror("Error binding listener socket");
    close(listen_fd);
    return NULL;
  }

  // 4. Start listening
  if (listen(listen_fd, 5) == -1) {
    perror("Error listening on socket");
    close(listen_fd);
    return NULL;
  }

  fprintf(stderr, "[INJECTOR] Socket listener thread is active at: %s. Waiting for data...\n", SOCKET_PATH);

  // 5. Accept a connection (This call blocks until a client connects)
  client_fd = accept(listen_fd, NULL, NULL);
  if (client_fd == -1) {
    perror("Error accepting connection");
    close(listen_fd);
    return NULL;
  }

  fprintf(stderr, "[INJECTOR] Client connected. Reading data...\n");

  // --- Data Reading Loop ---
  char buffer[BUF_SIZE];
  size_t total_read = 0;
  ssize_t bytes_read;
  received_data = NULL; // Start with NULL pointer

  while ((bytes_read = read(client_fd, buffer, BUF_SIZE)) > 0) {
    // Reallocate memory to hold the new data chunk
    char *new_ptr = realloc(received_data, total_read + bytes_read + 1); // +1 for null terminator
    if (new_ptr == NULL) {
      fprintf(stderr, "[INJECTOR] Memory allocation failed!\n");
      free(received_data);
      received_data = NULL;
      close(client_fd);
      close(listen_fd);
      unlink(SOCKET_PATH);
      return NULL;
    }
    received_data = new_ptr;

    // Copy the new data into the global buffer
    memcpy(received_data + total_read, buffer, bytes_read);
    total_read += bytes_read;
  }

  // Check for read error (bytes_read == -1) or successful EOF (bytes_read == 0)
  if (bytes_read == -1) {
    perror("[INJECTOR] Error during socket read");
    free(received_data);
    received_data = NULL;
  } else {
    // 6. Null-terminate the received string
    if (received_data) {
      received_data[total_read] = '\0';
    }

    // 7. Update global state
    data_received = true;

    fprintf(stderr, "[INJECTOR] Received %zu bytes of data successfully.\n", total_read);
    fprintf(stderr, "[INJECTOR] Data received state set to TRUE.\n");
  }

  // 8. Clean up
  close(client_fd);
  close(listen_fd);
  unlink(SOCKET_PATH); // Remove the socket file

  if (received_data) {
    fprintf(stderr, "%s\n", received_data);
  } else {
    fprintf(stderr, "Somehow a nullptr?");
  }

  return NULL;
}


/**
 * @brief The constructor function that runs before main().
 * * This is the entry point for the injected code. It launches the background thread.
 */
__attribute__((constructor))
  void pre_main_hook(void) {
    pthread_t tid;

    // Set up a detached thread so we don't need to manually join it later.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // Create the listener thread
    if (pthread_create(&tid, &attr, socket_listener, NULL) != 0) {
      fprintf(stderr, "[INJECTOR] Failed to create socket listener thread!\n");
    }

    pthread_attr_destroy(&attr);
    fprintf(stderr, "[INJECTOR] Pre-main hook executed. Listener thread started (TID: %lu).\n", (unsigned long)tid);
  }


/**
 * @brief Optional: A destructor to clean up the global string if main() exits.
 * * This is good practice for injected code.
 */
__attribute__((destructor))
  void post_main_cleanup(void) {
    if (received_data) {
      fprintf(stderr, "[INJECTOR] Post-main cleanup: Freeing received data.\n");
      free(received_data);
      received_data = NULL;
    }
  }
