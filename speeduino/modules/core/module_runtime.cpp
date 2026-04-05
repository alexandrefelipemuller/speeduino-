#include "module_runtime.h"

#include "config9_domains.h"
#include "module_interfaces.h"
#include "table_registry.h"
#include "tune_registry.h"

void core_modules_init_pre_pin_mapping(void)
{
  module_logging_init(configPage13);
}

void core_modules_init_post_pin_mapping(void)
{
  module_secondary_serial_init(get_secondary_serial_config(configPage9));
  module_comms_extended_init();
}

void core_modules_poll(void)
{
  module_secondary_serial_poll(get_secondary_serial_config(configPage9));
  module_comms_extended_poll(configPage9.enable_intcan, configPage2.canWBO);
}

void core_modules_on_engine_stop(void)
{
  module_advanced_engine_on_engine_stop(configPage4);
}

void core_modules_tick_50hz(void)
{
  module_comms_extended_tick_50hz();
}

void core_modules_tick_30hz(void)
{
  module_advanced_engine_tick_30hz();
  module_secondary_serial_tick_30hz(get_secondary_serial_config(configPage9));
  module_comms_extended_tick_30hz();
  module_logging_tick_30hz(configPage13);
}

void core_modules_tick_15hz(void)
{
  module_advanced_engine_tick_15hz();
  module_comms_extended_tick_15hz();
}

void core_modules_tick_10hz(void)
{
  module_advanced_engine_tick_10hz();
  module_comms_extended_tick_10hz();
  module_logging_tick_10hz(configPage13);
}

void core_modules_tick_4hz(uint8_t sensor_status, statuses &current)
{
  module_advanced_engine_tick_4hz();
  module_logging_tick_4hz(configPage13);
  module_comms_extended_tick_4hz(sensor_status, current, get_can_extended_config(configPage9));
}

void core_modules_tick_1hz(const statuses &current)
{
  module_logging_tick_1hz(current, configPage13);
}

void core_modules_apply_table_switching(statuses &current)
{
  module_table_switching_apply(configPage2, configPage10, fuelTable2, ignitionTable2, current);
}

statuses::scheduler_cut_t core_modules_get_scheduler_cut(statuses &current)
{
  return module_advanced_engine_scheduler_cut(current, configPage2, configPage4, configPage6, configPage9, configPage10);
}
