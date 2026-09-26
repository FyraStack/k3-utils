#include "configuration.hpp"

#include <cstdlib>
#include <stdexcept>

namespace configuration {

    int environment_int(const char *name, const int fallback) {
        if (const char *value = std::getenv(name); value != nullptr) {
            try {
                return std::stoi(value);
            } catch (const std::exception &) {
                throw std::runtime_error(std::string("Invalid integer in ") + name);
            }
        }
        return fallback;
    }

    std::string gpio_chip() {
        if (const char *value = std::getenv("GPIO_CHIP"); value != nullptr) {
            return value;
        }
        return "/dev/gpiochip0";
    }

    std::array<unsigned int, relay_count> relay_offsets() {
        // Wiring diagram: IN1..IN4 -> GPIO17, GPIO27, GPIO22, GPIO23.
        std::array<unsigned int, relay_count> offsets{};
        for (std::size_t i = 0; i < relay_count; ++i) {
            constexpr std::array<int, relay_count> default_offsets{17, 27, 22, 23};
            const auto name = std::string("RELAY_") + std::to_string(i + 1) + "_LINE";
            const int offset = environment_int(name.c_str(), default_offsets[i]);
            if (offset < 0) {
                throw std::runtime_error(name + " must not be negative");
            }
            offsets[i] = static_cast<unsigned int>(offset);
        }
        return offsets;
    }

} // namespace configuration
