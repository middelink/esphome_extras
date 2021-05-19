#include "max72xx.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

/*
 * this->buffer_ is layed out as 8(y) rows of num_chips(x/8) bytes, as the x data can be quickly shifted into a row of n-chips.
 */

namespace esphome {
namespace max72xx {

static const char TAG[] = "max72xx";

static const uint8_t MAX72XX_REGISTER_NOOP = 0x00;
static const uint8_t MAX72XX_REGISTER_DIGIT0 = 0x01;
static const uint8_t MAX72XX_REGISTER_DECODE_MODE = 0x09;
static const uint8_t MAX72XX_REGISTER_INTENSITY = 0x0A;
static const uint8_t MAX72XX_REGISTER_SCAN_LIMIT = 0x0B;
static const uint8_t MAX72XX_REGISTER_SHUTDOWN = 0x0C;
static const uint8_t MAX72XX_REGISTER_TEST = 0x0F;

float MAX72XXComponent::get_setup_priority() const { return setup_priority::PROCESSOR; }

void MAX72XXComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MAX72XX...");
  this->spi_setup();
  this->spi_buffer_ = new uint8_t[this->num_chips_ * 2];
  if (this->spi_buffer_ == nullptr) {
    ESP_LOGE(TAG, "Could not allocate spi buffer!");
    return;
  }
  init_internal_(this->num_chips_ * 8);

  // let's assume the user has 8x8 matrix displays connected
  this->control(MAX72XX_REGISTER_SCAN_LIMIT, 7);

  this->control(MAX72XX_REGISTER_TEST, 0);
  this->control(MAX72XX_REGISTER_DECODE_MODE, 0);
  this->control(MAX72XX_REGISTER_INTENSITY, this->intensity_);

  // Blank display
  this->clear();
  this->display();

  // power up (inverse logic, don't ask)
  this->control(MAX72XX_REGISTER_SHUTDOWN, 1);
}
void MAX72XXComponent::dump_config() {
  LOG_DISPLAY("", "MAX72XX", this);
  ESP_LOGCONFIG(TAG, "  Number of Chips: %u", this->num_chips_);
  ESP_LOGCONFIG(TAG, "  Intensity: %u", this->intensity_);
  ESP_LOGCONFIG(TAG, "  Chain direction: %s", (const char* []){"UP","DOWN","LEFT","RIGHT"}[this->chain_direction_]);
  ESP_LOGCONFIG(TAG, "  Swap column/rows: %s", ONOFF(this->hw_dig_rows_));
  ESP_LOGCONFIG(TAG, "  Reverse rows: %s", ONOFF(this->hw_rev_rows_));
  ESP_LOGCONFIG(TAG, "  Reverse columns: %s", ONOFF(this->hw_rev_cols_));
  LOG_PIN("  CS Pin: ", this->cs_);
  LOG_UPDATE_INTERVAL(this);
}

void MAX72XXComponent::update() {
  this->do_update_();
  this->display();
}

void MAX72XXComponent::fill(Color color) {
  memset(this->buffer_, color.is_on() ? 0xFF : 0, this->num_chips_ * 8);
}

void MAX72XXComponent::display() {
  // Write the buffer_ data out to all chips.
  uint8_t *ptr = this->buffer_;
  for (int row = 0; row < 8; row++) {
    for (int chip=0; chip < this->num_chips_; chip++) {
      this->spi_buffer_[chip*2] = MAX72XX_REGISTER_DIGIT0 + row;
      this->spi_buffer_[chip*2+1] = *ptr++;
    }
    this->enable();
    this->write_array(this->spi_buffer_, this->num_chips_ * 2);
    this->disable();
  }
}

void MAX72XXComponent::control(uint8_t reg, uint8_t param) {
  // Write the same control-param to all chips
  for (int chip=0; chip < this->num_chips_; chip++) {
    this->spi_buffer_[chip*2] = reg;
    this->spi_buffer_[chip*2+1] = param;
  }
  this->enable();
  this->write_array(this->spi_buffer_, this->num_chips_ * 2);
  this->disable();
}

#define SWAP(x,y) { x ^= y; y ^= x; x ^= y; }

void MAX72XXComponent::draw_absolute_pixel_internal(int x, int y, Color color) {
  //ESP_LOGD(TAG, "draw_absolute_pixel_internal(%d, %d, %d)", x, y, color);
  //TODO: add support for the chain_direction, swap row/col and rev_row, rev_col flags.

  switch (this->chain_direction_) {
    case CHAIN_DIRECTION_LEFT:
      y = 7u - y;
      break;
    case CHAIN_DIRECTION_RIGHT:
      x = (this->num_chips_ * 8u - 1) - x;
      break;
    case CHAIN_DIRECTION_UP:
      SWAP(x,y);
      break;
    case CHAIN_DIRECTION_DOWN:
      SWAP(x,y);
      x = (this->num_chips_ * 8u - 1) - x;
      y = 7u - y;
      break;
  }
  // Per-chip adjustments
  if (this->hw_dig_rows_) {
    int y1 = 7u-x;
    x = (x&~7u) | (y&7u);
    y = y1;
  }
  if (this->hw_rev_cols_)
    y = 7u-y;
  if (this->hw_rev_rows_)
    x = (x&~7u) | (7u-(x&7u));

  uint16_t offset = (y * this->num_chips_) + (x / 8);
  if (offset < this->num_chips_ * 8)
    this->buffer_[offset] |= 1 << (x&7u);
}

int MAX72XXComponent::get_height_internal() {
  switch (this->chain_direction_) {
    case CHAIN_DIRECTION_UP:
    case CHAIN_DIRECTION_DOWN:
      return this->num_chips_ * 8;
    case CHAIN_DIRECTION_LEFT:
    case CHAIN_DIRECTION_RIGHT:
    default:
      return 8;
  }
}

int MAX72XXComponent::get_width_internal() {
  switch (this->chain_direction_) {
    case CHAIN_DIRECTION_UP:
    case CHAIN_DIRECTION_DOWN:
      return 8;
    case CHAIN_DIRECTION_LEFT:
    case CHAIN_DIRECTION_RIGHT:
    default:
      return this->num_chips_ * 8;
  }
}

void MAX72XXComponent::set_writer(max72xx_writer_t &&writer) { this->writer_ = writer; }

void MAX72XXComponent::set_intensity(uint8_t intensity) {
  this->intensity_ = intensity;
  if (this->spi_buffer_) {
    this->control(MAX72XX_REGISTER_INTENSITY, this->intensity_);
  }
}


}  // namespace max72xx
}  // namespace esphome
