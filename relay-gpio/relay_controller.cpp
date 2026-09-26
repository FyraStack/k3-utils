#include "relay_controller.hpp"

#include "configuration.hpp"

#include <stdexcept>

RelayController::RelayController() :
    chip_(gpiod::chip(configuration::gpio_chip())),
    active_low_(configuration::environment_int("RELAY_ACTIVE_LOW", 1) != 0),
    contact_nc_(configuration::environment_int("RELAY_CONTACT_NC", 0) != 0), offsets_(configuration::relay_offsets()),
    request_(make_request()) {
    state_.fill(contact_nc_);
}

RelayController::~RelayController() {
    if (request_) {
        request_.release();
    }
}

gpiod::line_request RelayController::make_request() {
    const gpiod::line::offsets offsets(offsets_.begin(), offsets_.end());
    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::OUTPUT)
        .set_output_value(off_value() ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);

    auto builder = chip_.prepare_request();
    builder.set_consumer("relay-gpio").add_line_settings(offsets, settings);
    return builder.do_request();
}

void RelayController::set(const std::size_t index, const bool on) {
    if (index >= relay_count) {
        throw std::out_of_range("relay index");
    }
    request_.set_value(offsets_[index], value_for(on) ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
    state_[index] = on;
}

bool RelayController::get(const std::size_t index) const { return state_.at(index); }

int RelayController::value_for(const bool on) const { return on ? on_value() : off_value(); }

int RelayController::on_value() const {
    const bool energize = !contact_nc_;
    return energize == active_low_ ? 0 : 1;
}

int RelayController::off_value() const {
    const bool energize = contact_nc_;
    return energize == active_low_ ? 0 : 1;
}
