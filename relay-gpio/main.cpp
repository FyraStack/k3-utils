#include "http_server.hpp"
#include "relay_controller.hpp"

#include "configuration.hpp"

#include <iostream>

int main() {
    try {
        RelayController controller;
        const int port = configuration::environment_int("RELAY_HTTP_PORT", 8080);
        HttpServer server(port, controller);
        std::cout << "Relay server listening on port " << port << '\n';
        server.run();
    } catch (const std::exception &error) {
        std::cerr << "relay-gpio: " << error.what() << '\n';
        return 1;
    }
}
