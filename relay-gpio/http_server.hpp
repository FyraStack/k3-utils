#pragma once

#include <string>

class RelayController;

class HttpServer {
public:
    /**
     * Creates and binds an HTTP server for the supplied relay controller.
     *
     * @param port TCP port on which the server should listen.
     * @param controller Relay controller used by API requests. It must outlive this server.
     * @throws std::exception if the socket cannot be created or bound.
     */
    HttpServer(int port, RelayController &controller);

    /// Closes the listening socket.
    ~HttpServer();

    HttpServer(const HttpServer &) = delete;
    HttpServer &operator=(const HttpServer &) = delete;

    /**
     * Accepts and handles requests until the process is stopped.
     *
     * Supported routes are GET /, GET /api/relays, and
     * POST /api/relays/{1..4}/{on|off}.
     */
    void run();

private:
    void handle_client(int client);
    void send_response(int client, int status, const std::string &content_type, const std::string &body) const;
    std::string relay_state_json() const;

    int port_;
    int server_socket_ = -1;
    RelayController &controller_;
};
