#pragma once

#include <array>
#include <cstddef>

#include <gpiod.hpp>

class RelayController {
public:
    /// Number of relay outputs managed by this controller.
    static constexpr std::size_t relay_count = 4;

    /**
     * Opens the configured GPIO chip and requests all relay lines as outputs.
     *
     * All relays are initialized to their electrical off value. Configuration
     * is read from GPIO_CHIP, RELAY_ACTIVE_LOW, and RELAY_*_LINE variables.
     *
     * @throws std::exception if the GPIO chip or any relay line cannot be opened.
     */
    RelayController();

    /// Releases all GPIO lines owned by this controller.
    ~RelayController();

    RelayController(const RelayController &) = delete;
    RelayController &operator=(const RelayController &) = delete;

    /**
     * Changes the logical state of one relay.
     *
     * @param index Zero-based relay index, from 0 through relay_count - 1.
     * @param on True to energize the relay; false to de-energize it.
     * @throws std::out_of_range if index is outside the relay range.
     */
    void set(std::size_t index, bool on);

    /**
     * Returns the last logical state written to one relay.
     *
     * @param index Zero-based relay index, from 0 through relay_count - 1.
     * @return True if the relay is logically on, otherwise false.
     * @throws std::out_of_range if index is outside the relay range.
     */
    bool get(std::size_t index) const;

private:
    /// Converts logical on into the GPIO level required by relay polarity.
    int on_value() const;

    /// Converts logical off into the GPIO level required by relay polarity.
    int off_value() const;

    /// Creates a v2 GPIO line request for all configured relay offsets.
    gpiod::line_request make_request();

    gpiod::chip chip_;
    bool active_low_;
    std::array<unsigned int, relay_count> offsets_;
    gpiod::line_request request_;
    std::array<bool, relay_count> state_{};
};
