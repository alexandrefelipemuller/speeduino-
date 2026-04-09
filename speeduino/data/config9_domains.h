#pragma once

#include "support/bit_manip.h"
#include "data/config_pages.h"
#include "modules/secondary_serial/secondary_serial.h"

struct secondary_serial_config_t
{
  const config9 &page;

  uint8_t is_enabled() const { return page.enable_secondarySerial; }
  uint8_t protocol() const { return page.secondarySerialProtocol; }
  bool is_legacy_fixed_protocol() const
  {
    return (page.secondarySerialProtocol == SECONDARY_SERIAL_PROTO_GENERIC_FIXED);
  }
  bool is_msdroid_protocol() const
  {
    return (page.secondarySerialProtocol == SECONDARY_SERIAL_PROTO_MSDROID);
  }
  uint8_t caninput_start_byte(uint8_t channel) const
  {
    return page.caninput_source_start_byte[channel];
  }
  bool caninput_is_two_bytes(uint8_t channel) const
  {
    return BIT_CHECK(page.caninput_source_num_bytes, channel) > 0U;
  }
};

struct can_extended_config_t
{
  const config9 &page;

  uint8_t is_internal_can_available() const { return page.intcan_available; }
  uint8_t is_internal_can_enabled() const { return page.enable_intcan; }
  uint8_t is_secondary_serial_enabled() const { return page.enable_secondarySerial; }
  uint8_t speeduino_ts_can_id() const { return page.speeduino_tsCanId; }
  uint16_t obd_address() const { return page.obd_address; }
  uint16_t caninput_source_address(uint8_t channel) const
  {
    return page.caninput_source_can_address[channel];
  }
  uint8_t caninput_selection(uint8_t channel) const
  {
    return page.caninput_sel[channel];
  }
  uint8_t caninput_start_byte(uint8_t channel) const
  {
    return page.caninput_source_start_byte[channel];
  }
  bool caninput_is_two_bytes(uint8_t channel) const
  {
    return BIT_CHECK(page.caninput_source_num_bytes, channel) > 0U;
  }
  uint8_t caninput_endianness() const { return page.caninputEndianess; }
  uint8_t aux_input_analog_pin(uint8_t channel) const { return page.Auxinpina[channel]; }
  uint8_t aux_input_digital_pin(uint8_t channel) const { return page.Auxinpinb[channel]; }
};

inline secondary_serial_config_t get_secondary_serial_config(const config9 &page)
{
  return { page };
}

inline can_extended_config_t get_can_extended_config(const config9 &page)
{
  return { page };
}
