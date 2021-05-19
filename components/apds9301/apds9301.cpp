#include "apds9301.h"
#include "esphome/core/log.h"

namespace esphome {
namespace apds9301 {

static const char *TAG = "apds9301.sensor";

static const uint8_t APDS9301_REGISTER_CONTROL = 0x80;
static const uint8_t APDS9301_REGISTER_TIMING = 0x81;
static const uint8_t APDS9301_REGISTER_ID = 0x8A;
static const uint8_t APDS9301_REGISTER_ADC = 0x8C | 0x20; // Use word transfers.

static const uint8_t APDS9301_TIMING_BITS_LOOKUP_TABLE[] = {0x10, 0x11, 0x12, 0x00, 0x01, 0x02};
static const uint32_t APDS9301_TIMING_MS_LOOKUP_TABLE[] PROGMEM = {402+5, 101+5, 14+5, 402+5, 101+5, 14+5};

void APDS9301Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up APDS9301 '%s'...", this->name_.c_str());
  uint8_t chip_id;
  if (!this->read_byte(APDS9301_REGISTER_ID, &chip_id) || (chip_id & 0xF0) != 0x50) {
    this->mark_failed();
    return;
  }
  // Power up
  if (!this->write_byte(APDS9301_REGISTER_CONTROL, 0x03)) {
    this->mark_failed();
    return;
  }
}
void APDS9301Sensor::dump_config() {
  LOG_SENSOR("", "APDS9301", this);
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with APDS9301 failed!");
  }

  static const char *scales[] PROGMEM = {"16", "4.024", "0.546", "1", "0.252", "0.0034"};
  ESP_LOGCONFIG(TAG, "  Resolution: %sx", scales[this->resolution_]);
  LOG_UPDATE_INTERVAL(this);
}

void APDS9301Sensor::update() {
  if (!this->write_byte(APDS9301_REGISTER_TIMING, APDS9301_TIMING_BITS_LOOKUP_TABLE[this->resolution_])) {
    this->status_set_warning();
    return;
  }

  this->set_timeout("illuminance", APDS9301_TIMING_MS_LOOKUP_TABLE[this->resolution_], [this]() { this->read_data_(); });
}

float APDS9301Sensor::get_setup_priority() const { return setup_priority::DATA; }

void APDS9301Sensor::read_data_() {
  // The APDS-9301 has the registers in LOW, HIGH order, same as the ESP CPU.
  // Therefore we can read the byte values directly into the uint16 variables.
  uint16_t raw_value[2];
  if (!this->read_bytes(APDS9301_REGISTER_ADC, (uint8_t*)raw_value, 4)) {
    this->status_set_warning();
    return;
  }
  ESP_LOGVV(TAG, "adc_values: %04x, %04x", raw_value[0], raw_value[1]);

  // Verify if the data makes sense
  switch (this->resolution_) {
    case APDS9301_RESOLUTION_4P024_LX:
    case APDS9301_RESOLUTION_0P252_LX:
            if (raw_value[0] > 5047 || raw_value[1] > 5047) {
	      this->status_set_warning();
	      return;
	    }
	    break;
    case APDS9301_RESOLUTION_0P546_LX:
    case APDS9301_RESOLUTION_0P034_LX:
            if (raw_value[0] > 37177 || raw_value[1] > 37177) {
	      this->status_set_warning();
	      return;
	    }
	    break;
    case APDS9301_RESOLUTION_16P00_LX:
    case APDS9301_RESOLUTION_1P000_LX:
	    break;
  }

  float ch0 = raw_value[0];
  float ch1 = raw_value[1];
  float ratio = ch1 / ch0;
  switch (this->resolution_) {
    case APDS9301_RESOLUTION_0P252_LX:
	    ch0 *= 16;
	    ch1 *= 16;
	    /* fall through */
    case APDS9301_RESOLUTION_4P024_LX:
	    ch0 /= 0.252;
	    ch1 /= 0.252;
	    break;
    case APDS9301_RESOLUTION_0P034_LX:
	    ch0 *= 16;
	    ch1 *= 16;
	    /* fall through */
    case APDS9301_RESOLUTION_0P546_LX:
	    ch0 /= 0.034;
	    ch1 /= 0.034;
	    break;
    case APDS9301_RESOLUTION_1P000_LX:
	    ch0 *= 16;
	    ch1 *= 16;
	    /* fall through */
    case APDS9301_RESOLUTION_16P00_LX:
	    break;
  }
  float lx = 0.0;
  if (ratio <= 0.5) {
	  lx = (0.0304 * ch0) - (0.062 * ch0 * pow((ch1/ch0), 1.4));
  } else if (ratio <= 0.61) {
	  lx = (0.0224 * ch0) - (0.031 * ch1);
  } else if (ratio <= 0.8) {
	  lx = (0.0128 * ch0) - (0.0153 * ch1);
  } else if (ratio <= 1.3) {
	  lx = (0.00146 * ch0) - (0.00112*ch1);
  }

  ESP_LOGD(TAG, "'%s': Got illuminance=%.1flx", this->get_name().c_str(), lx);
  this->publish_state(lx);
  this->status_clear_warning();
}
void APDS9301Sensor::set_resolution(APDS9301Resolution resolution) { this->resolution_ = resolution; }

}  // namespace bh1750
}  // namespace esphome
