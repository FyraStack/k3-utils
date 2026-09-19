#include "relay_controller.hpp"

#include "configuration.hpp"

#include <stdexcept>

RelayController::RelayController()
    : chip_(gpiod::chip(configuration::gpio_chip())),
      active_low_(configuration::environment_int("RELAY_ACTIVE_LOW", 1) != 0),
      offsets_(configuration::relay_offsets()), request_(make_request()) {}

RelayController::~RelayController() {
  if (request_) {
    request_.release();
  }
}

gpiod::line_request RelayController::make_request() {
  gpiod::line::offsets offsets(offsets_.begin(), offsets_.end());
  gpiod::line_settings settings;
  settings.set_direction(gpiod::line::direction::OUTPUT)
      .set_output_value(off_value() ? gpiod::line::value::ACTIVE
                                    : gpiod::line::value::INACTIVE);

  auto builder = chip_.prepare_request();
  builder.set_consumer("relay-gpio").add_line_settings(offsets, settings);
  return builder.do_request();
}

void RelayController::set(std::size_t index, bool on) {
  if (index >= relay_count) {
    throw std::out_of_range("relay index");
  }
  request_.set_value(offsets_[index],
                     on ? (on_value() ? gpiod::line::value::ACTIVE
                                      : gpiod::line::value::INACTIVE)
                        : (off_value() ? gpiod::line::value::ACTIVE
                                       : gpiod::line::value::INACTIVE));
  state_[index] = on;
}

bool RelayController::get(std::size_t index) const { return state_.at(index); }

int RelayController::on_value() const { return active_low_ ? 0 : 1; }

int RelayController::off_value() const { return active_low_ ? 1 : 0; }
