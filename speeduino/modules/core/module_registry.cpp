#include "module_registry.h"

#include "data/config9_domains.h"
#include "module_interfaces.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"

namespace {
static void hook_logging_init_pre_pin_mapping(module_runtime_context_t &)
{
  module_logging_init(configPage13);
}

static void hook_secondary_serial_init_post_pin_mapping(module_runtime_context_t &)
{
  module_secondary_serial_init(get_secondary_serial_config(configPage9));
}

static void hook_comms_extended_init_post_pin_mapping(module_runtime_context_t &)
{
  module_comms_extended_init();
}

static void hook_secondary_serial_poll(module_runtime_context_t &)
{
  module_secondary_serial_poll(get_secondary_serial_config(configPage9));
}

static void hook_comms_extended_poll(module_runtime_context_t &)
{
  module_comms_extended_poll(configPage9.enable_intcan, configPage2.canWBO);
}

static void hook_advanced_engine_on_engine_stop(module_runtime_context_t &)
{
  module_advanced_engine_on_engine_stop(configPage4);
}

static void hook_comms_extended_tick_50hz(module_runtime_context_t &)
{
  module_comms_extended_tick_50hz();
}

static void hook_advanced_engine_tick_30hz(module_runtime_context_t &)
{
  module_advanced_engine_tick_30hz();
}

static void hook_secondary_serial_tick_30hz(module_runtime_context_t &)
{
  module_secondary_serial_tick_30hz(get_secondary_serial_config(configPage9));
}

static void hook_comms_extended_tick_30hz(module_runtime_context_t &)
{
  module_comms_extended_tick_30hz();
}

static void hook_logging_tick_30hz(module_runtime_context_t &)
{
  module_logging_tick_30hz(configPage13);
}

static void hook_advanced_engine_tick_15hz(module_runtime_context_t &)
{
  module_advanced_engine_tick_15hz();
}

static void hook_comms_extended_tick_15hz(module_runtime_context_t &)
{
  module_comms_extended_tick_15hz();
}

static void hook_advanced_engine_tick_10hz(module_runtime_context_t &)
{
  module_advanced_engine_tick_10hz();
}

static void hook_comms_extended_tick_10hz(module_runtime_context_t &)
{
  module_comms_extended_tick_10hz();
}

static void hook_logging_tick_10hz(module_runtime_context_t &)
{
  module_logging_tick_10hz(configPage13);
}

static void hook_advanced_engine_tick_4hz(module_runtime_context_t &)
{
  module_advanced_engine_tick_4hz();
}

static void hook_logging_tick_4hz(module_runtime_context_t &)
{
  module_logging_tick_4hz(configPage13);
}

static void hook_comms_extended_tick_4hz(module_runtime_context_t &context)
{
  module_comms_extended_tick_4hz(context.sensor_status, *context.current_status, get_can_extended_config(configPage9));
}

static void hook_logging_tick_1hz(module_runtime_context_t &context)
{
  module_logging_tick_1hz(*context.current_status_const, configPage13);
}

static void hook_apply_table_switching(statuses &current)
{
  module_table_switching_apply(configPage2, configPage10, fuelTable2, ignitionTable2, current);
}

static statuses::scheduler_cut_t hook_advanced_engine_scheduler_cut(statuses &current)
{
  return module_advanced_engine_scheduler_cut(current, configPage2, configPage4, configPage6, configPage9, configPage10);
}

static constexpr module_hook_descriptor_t coreHooks[] = {};

static constexpr module_hook_descriptor_t advancedEngineHooks[] = {
  { module_hook_phase_t::on_engine_stop, hook_advanced_engine_on_engine_stop },
  { module_hook_phase_t::tick_30hz, hook_advanced_engine_tick_30hz },
  { module_hook_phase_t::tick_15hz, hook_advanced_engine_tick_15hz },
  { module_hook_phase_t::tick_10hz, hook_advanced_engine_tick_10hz },
  { module_hook_phase_t::tick_4hz, hook_advanced_engine_tick_4hz },
};

static constexpr module_hook_descriptor_t tableSwitchingHooks[] = {};

static constexpr module_hook_descriptor_t secondarySerialHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_secondary_serial_init_post_pin_mapping },
  { module_hook_phase_t::poll, hook_secondary_serial_poll },
  { module_hook_phase_t::tick_30hz, hook_secondary_serial_tick_30hz },
};

static constexpr module_hook_descriptor_t commsExtendedHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_comms_extended_init_post_pin_mapping },
  { module_hook_phase_t::poll, hook_comms_extended_poll },
  { module_hook_phase_t::tick_50hz, hook_comms_extended_tick_50hz },
  { module_hook_phase_t::tick_30hz, hook_comms_extended_tick_30hz },
  { module_hook_phase_t::tick_15hz, hook_comms_extended_tick_15hz },
  { module_hook_phase_t::tick_10hz, hook_comms_extended_tick_10hz },
  { module_hook_phase_t::tick_4hz, hook_comms_extended_tick_4hz },
};

static constexpr module_hook_descriptor_t loggingHooks[] = {
  { module_hook_phase_t::init_pre_pin_mapping, hook_logging_init_pre_pin_mapping },
  { module_hook_phase_t::tick_30hz, hook_logging_tick_30hz },
  { module_hook_phase_t::tick_10hz, hook_logging_tick_10hz },
  { module_hook_phase_t::tick_4hz, hook_logging_tick_4hz },
  { module_hook_phase_t::tick_1hz, hook_logging_tick_1hz },
};

static const module_descriptor_t registeredModules[] = {
  { getCorePageDescriptors(), getCoreStorageMaps(), { coreHooks, _countof(coreHooks) }, nullptr, nullptr },
  { getAdvancedEnginePageDescriptors(), getAdvancedEngineStorageMaps(), { advancedEngineHooks, _countof(advancedEngineHooks) }, nullptr, hook_advanced_engine_scheduler_cut },
  { getTableSwitchingPageDescriptors(), getTableSwitchingStorageMaps(), { tableSwitchingHooks, _countof(tableSwitchingHooks) }, hook_apply_table_switching, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { secondarySerialHooks, _countof(secondarySerialHooks) }, nullptr, nullptr },
  { getCommsExtendedPageDescriptors(), getCommsExtendedStorageMaps(), { commsExtendedHooks, _countof(commsExtendedHooks) }, nullptr, nullptr },
  { getLoggingPageDescriptors(), getLoggingStorageMaps(), { loggingHooks, _countof(loggingHooks) }, nullptr, nullptr },
};
} // namespace

const module_descriptor_t *getRegisteredModules()
{
  return registeredModules;
}

uint8_t getRegisteredModuleCount()
{
  return _countof(registeredModules);
}
