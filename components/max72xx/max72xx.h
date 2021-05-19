#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace max72xx {

enum ChainDirection {
  CHAIN_DIRECTION_UP = 0, // next chip is above the prev chip
  CHAIN_DIRECTION_DOWN = 1, // next chip is below the prev chip
  CHAIN_DIRECTION_LEFT = 2, // next chip is to the left of the prev chip
  CHAIN_DIRECTION_RIGHT = 3, // next chip is to the right of the prev chip
};

class MAX72XXComponent;

using max72xx_writer_t = std::function<void(MAX72XXComponent &)>;

class MAX72XXComponent : public PollingComponent,
                         public display::DisplayBuffer,
                         public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                               spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1MHZ> {
 public:
  explicit MAX72XXComponent(int num_chips, ChainDirection direction, bool swap_rows, bool rev_rows, bool rev_cols)
        : num_chips_(num_chips), chain_direction_(direction), hw_dig_rows_(swap_rows),
	  hw_rev_rows_(rev_rows), hw_rev_cols_(rev_cols) {}

  void set_writer(max72xx_writer_t &&writer);

  void setup() override;

  void dump_config() override;

  void update() override;

  void fill(Color color) override;

  float get_setup_priority() const override;

  void set_intensity(uint8_t intensity);

  void draw_absolute_pixel_internal(int x, int y, Color color) override;

 protected:
  int get_height_internal() override;
  int get_width_internal() override;

  void display();
  void control(uint8_t reg, uint8_t value);

  uint8_t num_chips_;
  ChainDirection chain_direction_;
  bool hw_dig_rows_;
  bool hw_rev_rows_;
  bool hw_rev_cols_;
  uint8_t intensity_{15};  /// Intensity of the display from 0 to 15 (most)
  uint8_t *spi_buffer_{nullptr};
  optional<max72xx_writer_t> writer_{};
};

}  // namespace max72xx
}  // namespace esphome
