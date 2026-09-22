#include "http_server.hpp"

#include "relay_controller.hpp"

#include <cstdint>

#include <sstream>
#include <stdexcept>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

HttpServer::HttpServer(int port, RelayController &controller) : port_(port), controller_(controller) {
    server_socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        throw std::runtime_error("Could not create HTTP socket");
    }

    int reuse = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<uint16_t>(port_));
    if (bind(server_socket_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0 ||
        listen(server_socket_, 8) < 0) {
        ::close(server_socket_);
        server_socket_ = -1;
        throw std::runtime_error("Could not bind/listen on HTTP port " + std::to_string(port_));
    }
}

HttpServer::~HttpServer() {
    if (server_socket_ >= 0) {
        ::close(server_socket_);
    }
}

void HttpServer::run() {
    while (true) {
        const int client = accept(server_socket_, nullptr, nullptr);
        if (client >= 0) {
            handle_client(client);
            ::close(client);
        }
    }
}

std::string HttpServer::relay_state_json() const {
    std::ostringstream body;
    body << "{\"relays\":[";
    for (std::size_t i = 0; i < RelayController::relay_count; ++i) {
        if (i != 0) {
            body << ',';
        }
        body << "{\"id\":" << (i + 1) << ",\"on\":" << (controller_.get(i) ? "true" : "false") << '}';
    }
    body << "]}";
    return body.str();
}

void HttpServer::send_response(int client, int status, const std::string &content_type, const std::string &body) const {
    const std::string response = "HTTP/1.1 " + std::to_string(status) + (status == 200 ? " OK" : " Bad Request") +
        "\r\nContent-Type: " + content_type + "\r\nContent-Length: " + std::to_string(body.size()) +
        "\r\nConnection: close\r\n\r\n" + body;
    (void) ::send(client, response.data(), response.size(), 0);
}

void HttpServer::handle_client(int client) {
    char request[4096]{};
    const ssize_t received = ::recv(client, request, sizeof(request) - 1, 0);
    if (received <= 0) {
        return;
    }

    std::istringstream line(std::string(request, static_cast<std::size_t>(received)));
    std::string method, path, version;
    line >> method >> path >> version;

    try {
        if (method == "GET" && path == "/api/relays") {
            send_response(client, 200, "application/json", relay_state_json());
            return;
        }

        const std::string prefix = "/api/relays/";
        if (method == "POST" && path.rfind(prefix, 0) == 0) {
            const std::string command = path.substr(prefix.size());
            const auto separator = command.find('/');
            if (separator == std::string::npos) {
                throw std::runtime_error("bad path");
            }
            const auto id = std::stoul(command.substr(0, separator));
            const auto action = command.substr(separator + 1);
            if (id < 1 || id > RelayController::relay_count || (action != "on" && action != "off")) {
                throw std::runtime_error("bad relay command");
            }
            controller_.set(id - 1, action == "on");
            send_response(client, 200, "application/json", relay_state_json());
            return;
        }

        if (method == "GET" && path == "/healthz") {
            send_response(client, 200, "text/plain", "ok\n");
            return;
        }
        send_response(client, 400, "text/plain", "Bad request\n");
    } catch (const std::exception &error) {
        send_response(client, 400, "text/plain", std::string("Error: ") + error.what() + "\n");
    }
}
