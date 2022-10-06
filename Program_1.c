// EECE 446
// 9/14/2022
// Andrew Roda, Clemens Stigl
// Program 1
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int lookup_and_connect(const char *host, const char *service, int buffersize);

int main(int argc, char *argv[]) {
  int buffsize = atoi(argv[1]);
  int s;
  const char *host = "www.ecst.csuchico.edu";
  const char *port = "80";

  /* Lookup IP and connect to server */
  if ((s = lookup_and_connect(host, port, buffsize)) < 0) {
    exit(1);
  }

  /* Modify the program so it
   *
   * 1) connects to www.ecst.csuchico.edu on port 80 (mostly done above)
   * 2) sends "GET /~kkredo/file.html HTTP/1.0\r\n\r\n" to the server
   * 3) receives all the data sent by the server (HINT: "orderly shutdown" in
   * recv(2)) 4) prints the total number of bytes received
   *
   * */

  close(s);

  return 0;
}

int lookup_and_connect(const char *host, const char *service, int buffsize) {
  struct addrinfo hints;
  struct addrinfo *rp, *result;
  int s;

  /* Translate host name into peer's IP address */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
    fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* Iterate through the address list and try to connect */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
      continue;
    }

    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(s);
  }
  if (rp == NULL) {
    perror("stream-talk-client: connect");
    return -1;
  }
  char *message;
  int len;
  message = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
  len = strlen(message);
  if (send(s, message, len, 0) < 0) {
    perror("Send failed");
    return -1;
  }
  char buf[buffsize];
  // printf("Sent Message\n");
  ssize_t brec = 0; // bytes recieved
  ssize_t temp = 1;
  int count = 0;
  while (temp != 0 && temp != -1) {
    temp = recv(s, buf, buffsize, 0);
    if (temp == -1) {
      perror("Error!\n");
      return -1;
    }
    // printf(
    // "%s\n======================================================================================================\n",
    // buf);

    for (int i = 0; i < buffsize - 3; i++) {
      if (strstr(buf + i, "<h1>") == buf + i) {
        count++;
        i = i + 4;
      }
    }
    brec = brec + temp;
  }
  brec--;
  printf("Number of <h1> tags: %d\nNumber of bytes: %zu\n", count, brec);
  freeaddrinfo(result);

  return s;
}