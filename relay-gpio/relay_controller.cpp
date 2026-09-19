#include "relay_controller.hpp"

#include "configuration.hpp"

#include <stdexcept>

RelayController::RelayController() :
    chip_(gpiod::chip(configuration::gpio_chip())),
    active_low_(configuration::environment_int("RELAY_ACTIVE_LOW", 1) != 0), offsets_(configuration::relay_offsets()) {
    for (std::size_t i = 0; i < relay_count; ++i) {
        lines_[i] = chip_.get_line(offsets_[i]);
        lines_[i].request({"relay-gpio", gpiod::line_request::DIRECTION_OUTPUT, 0}, off_value());
    }
}

RelayController::~RelayController() {
    for (auto &line: lines_) {
        if (line) {
            line.release();
        }
    }
}

void RelayController::set(std::size_t index, bool on) {
    if (index >= relay_count) {
        throw std::out_of_range("relay index");
    }
    lines_[index].set_value(on ? on_value() : off_value());
    state_[index] = on;
}

bool RelayController::get(std::size_t index) const { return state_.at(index); }

int RelayController::on_value() const { return active_low_ ? 0 : 1; }

int RelayController::off_value() const { return active_low_ ? 1 : 0; }
