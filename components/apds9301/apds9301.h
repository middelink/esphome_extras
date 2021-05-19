#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace apds9301 {

/// Enum listing all resolutions that can be used with the APDS9301
enum APDS9301Resolution {
  APDS9301_RESOLUTION_16P00_LX = 0,  // 322 / 322 * 16
  APDS9301_RESOLUTION_4P024_LX = 1,  //  81 / 322 * 16
  APDS9301_RESOLUTION_0P546_LX = 2,  //  11 / 322 * 16
  APDS9301_RESOLUTION_1P000_LX = 3,  // 322 / 322
  APDS9301_RESOLUTION_0P252_LX = 4,  //  81 / 322
  APDS9301_RESOLUTION_0P034_LX = 5,  //  11 / 322
};

/// This class implements support for the i2c-based APDS9301 ambient light sensor.
class APDS9301Sensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
 public:
  /** Set the resolution of this sensor.
   *
   * Possible values are:
   *
   *  -  APDS9301_RESOLUTION_16P00_LX,
   *  -  APDS9301_RESOLUTION_4P024_LX,
   *  -  APDS9301_RESOLUTION_0P546_LX,
   *  -  APDS9301_RESOLUTION_1P000_LX,  (default)
   *  -  APDS9301_RESOLUTION_0P252_LX,
   *  -  APDS9301_RESOLUTION_0P034_LX,
   *
   * @param resolution The new resolution of the sensor.
   */
  void set_resolution(APDS9301Resolution scale);

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  void setup() override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override;

 protected:
  void read_data_();

  APDS9301Resolution resolution_{APDS9301_RESOLUTION_1P000_LX};
};

}  // namespace apds9301
}  // namespace esphome
