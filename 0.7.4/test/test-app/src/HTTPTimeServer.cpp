//
// HTTPTimeServer.cpp
//
// A minimal HTTP time server using POSIX sockets.
// Returns the current date and time on every request to port 8080.
// No external library dependencies required.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

static volatile sig_atomic_t running = 1;

static void handle_signal(int) { running = 0; }

static void *handle_client(void *arg)
{
    int client_fd = *static_cast<int *>(arg);
    delete static_cast<int *>(arg);

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    char body[512];
    int body_len = snprintf(body, sizeof(body),
        "<html><head><title>HTTPTimeServer</title>"
        "<meta http-equiv=\"refresh\" content=\"1\"></head>"
        "<body><p style=\"text-align:center;font-size:48px;\">%s</p></body></html>",
        timebuf);

    char response[1024];
    int resp_len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n%s",
        body_len, body);

    write(client_fd, response, resp_len);
    close(client_fd);
    return NULL;
}

int main()
{
    signal(SIGINT,  handle_signal);
    signal(SIGTERM, handle_signal);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }
    if (listen(server_fd, 128) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    fprintf(stderr, "HTTPTimeServer listening on port 8080\n");

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        int *fd_ptr = new int(client_fd);
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, fd_ptr) != 0) {
            perror("pthread_create");
            close(client_fd);
            delete fd_ptr;
        } else {
            pthread_detach(tid);
        }
    }

    close(server_fd);
    fprintf(stderr, "HTTPTimeServer stopped\n");
    return 0;
}
