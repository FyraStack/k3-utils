#pragma once

#include <array>
#include <cstddef>
#include <string>

namespace configuration {

    /// Number of relay outputs supported by the application.
    constexpr std::size_t relay_count = 4;

    /**
     * Reads an integer environment variable or returns a default value.
     *
     * @param name Environment variable name.
     * @param fallback Value returned when the variable is not set.
     * @return The parsed integer value.
     * @throws std::runtime_error if the variable is set but is not an integer.
     */
    int environment_int(const char *name, int fallback);

    /// Returns the GPIO chip path from GPIO_CHIP, defaulting to /dev/gpiochip0.
    /// @return GPIO character-device path.
    std::string gpio_chip();

    /**
     * Reads the four relay line offsets from RELAY_1_LINE through RELAY_4_LINE.
     *
     * @return GPIO line offsets in relay-number order.
     * @throws std::runtime_error if an offset is negative or invalid.
     */
    std::array<unsigned int, relay_count> relay_offsets();

} // namespace configuration
