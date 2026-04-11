#include "module_registry.h"

#include "data/config9_domains.h"
#include "data/pin_registry.h"
#include "module_interfaces.h"
#include "modules/boost/boost.h"
#include "modules/engine_protection/engine_protection.h"
#include "modules/fan_aircon/module_fan_aircon.h"
#include "modules/launch_flatshift/launch_flatshift.h"
#include "modules/nitrous/module_nitrous.h"
#include "modules/programmable_io/programmable_io.h"
#include "modules/vvt/vvt.h"
#include "modules/wmi/wmi.h"
#include "modules/wmi/wmi_storage.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"

namespace {
static void hook_logging_init_pre_pin_mapping(module_runtime_context_t &)
{
  module_logging_init(configPage13);
}

static void hook_boost_init_post_pin_mapping(module_runtime_context_t &)
{
  initialiseBoost(pinBoost);
}

static void hook_vvt_init_post_pin_mapping(module_runtime_context_t &)
{
  initialiseVVT(pinVVT_1, pinVVT_2);
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

static void hook_boost_on_engine_stop(module_runtime_context_t &)
{
  boostDisable();
}

static void hook_fan_aircon_init_post_pin_mapping(module_runtime_context_t &)
{
  module_fan_aircon_init(pinFan);
}

static void hook_fan_aircon_tick_10hz(module_runtime_context_t &)
{
  module_fan_aircon_tick_10hz();
}

static void hook_fan_aircon_tick_1hz(module_runtime_context_t &)
{
  module_fan_aircon_tick_1hz();
}

static void hook_programmable_io_init_post_pin_mapping(module_runtime_context_t &)
{
  module_programmable_io_init_post_pin_mapping();
}

static void hook_programmable_io_tick_10hz(module_runtime_context_t &)
{
  module_programmable_io_tick_10hz();
}

static void hook_nitrous_init_post_pin_mapping(module_runtime_context_t &)
{
  module_nitrous_init_post_pin_mapping();
}

static void hook_vvt_on_engine_stop(module_runtime_context_t &)
{
  vvt1Off();
  vvt2Off();
  DISABLE_VVT_TIMER();
}

static void hook_comms_extended_tick_50hz(module_runtime_context_t &)
{
  module_comms_extended_tick_50hz();
}

static void hook_boost_tick_30hz(module_runtime_context_t &)
{
  boostControl();
}

static void hook_vvt_tick_30hz(module_runtime_context_t &)
{
  vvtControl();
}

static statuses::scheduler_cut_t hook_engine_protection_scheduler_cut(statuses &current)
{
  return module_engine_protection_scheduler_cut(current, configPage2, configPage4, configPage6, configPage9, configPage10);
}

static void hook_wmi_tick_30hz(module_runtime_context_t &)
{
  module_wmi_tick_30hz();
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

static void hook_comms_extended_tick_15hz(module_runtime_context_t &)
{
  module_comms_extended_tick_15hz();
}

static void hook_launch_flatshift_tick_15hz(module_runtime_context_t &)
{
  module_launch_flatshift_tick_15hz();
}

static void hook_nitrous_tick_4hz(module_runtime_context_t &)
{
  module_nitrous_tick_4hz();
}

static void hook_comms_extended_tick_10hz(module_runtime_context_t &)
{
  module_comms_extended_tick_10hz();
}

static void hook_advanced_engine_tick_10hz(module_runtime_context_t &)
{
  module_advanced_engine_tick_10hz();
}

static void hook_logging_tick_10hz(module_runtime_context_t &)
{
  module_logging_tick_10hz(configPage13);
}

static void hook_comms_extended_tick_4hz(module_runtime_context_t &context)
{
  module_comms_extended_tick_4hz(context.sensor_status, *context.current_status, get_can_extended_config(configPage9));
}

static void hook_logging_tick_4hz(module_runtime_context_t &)
{
  module_logging_tick_4hz(configPage13);
}

static void hook_logging_tick_1hz(module_runtime_context_t &context)
{
  module_logging_tick_1hz(*context.current_status_const, configPage13);
}

static void hook_apply_table_switching(statuses &current)
{
  module_table_switching_apply(configPage2, configPage10, fuelTable2, ignitionTable2, current);
}

static constexpr module_hook_descriptor_t coreHooks[] = {};

static constexpr module_hook_descriptor_t boostHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_boost_init_post_pin_mapping },
  { module_hook_phase_t::on_engine_stop, hook_boost_on_engine_stop },
  { module_hook_phase_t::tick_30hz, hook_boost_tick_30hz },
};

static constexpr module_page_storage_descriptor_t boostPageStorage[] = {};

static constexpr module_hook_descriptor_t vvtHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_vvt_init_post_pin_mapping },
  { module_hook_phase_t::on_engine_stop, hook_vvt_on_engine_stop },
  { module_hook_phase_t::tick_30hz, hook_vvt_tick_30hz },
};

static constexpr module_hook_descriptor_t engineProtectionHooks[] = {};

static constexpr module_hook_descriptor_t launchFlatShiftHooks[] = {
  { module_hook_phase_t::tick_15hz, hook_launch_flatshift_tick_15hz },
};

static constexpr module_hook_descriptor_t fanAirconHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_fan_aircon_init_post_pin_mapping },
  { module_hook_phase_t::tick_10hz, hook_fan_aircon_tick_10hz },
  { module_hook_phase_t::tick_1hz, hook_fan_aircon_tick_1hz },
};

static constexpr module_hook_descriptor_t programmableIoHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_programmable_io_init_post_pin_mapping },
  { module_hook_phase_t::tick_10hz, hook_programmable_io_tick_10hz },
};

static constexpr module_hook_descriptor_t nitrousHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_nitrous_init_post_pin_mapping },
  { module_hook_phase_t::tick_4hz, hook_nitrous_tick_4hz },
};

static constexpr module_hook_descriptor_t wmiHooks[] = {
  { module_hook_phase_t::tick_30hz, hook_wmi_tick_30hz },
};

static constexpr module_page_storage_descriptor_t wmiPageStorage[] = {
  { wmiMapPage, module_wmi_save_pages, module_wmi_load_pages },
};

static constexpr module_hook_descriptor_t advancedEngineHooks[] = {
  { module_hook_phase_t::tick_10hz, hook_advanced_engine_tick_10hz },
  { module_hook_phase_t::on_engine_stop, hook_advanced_engine_on_engine_stop },
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
  { getCorePageDescriptors(), getCoreStorageMaps(), { nullptr, 0U }, { coreHooks, _countof(coreHooks) }, nullptr, nullptr },
  { getBoostPageDescriptors(), getBoostStorageMaps(), { boostPageStorage, _countof(boostPageStorage) }, { boostHooks, _countof(boostHooks) }, nullptr, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { vvtHooks, _countof(vvtHooks) }, nullptr, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { engineProtectionHooks, _countof(engineProtectionHooks) }, nullptr, hook_engine_protection_scheduler_cut },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { launchFlatShiftHooks, _countof(launchFlatShiftHooks) }, nullptr, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { fanAirconHooks, _countof(fanAirconHooks) }, nullptr, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { programmableIoHooks, _countof(programmableIoHooks) }, nullptr, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { nitrousHooks, _countof(nitrousHooks) }, nullptr, nullptr },
  { getWmiPageDescriptors(), getWmiStorageMaps(), { wmiPageStorage, _countof(wmiPageStorage) }, { wmiHooks, _countof(wmiHooks) }, nullptr, nullptr },
  { getAdvancedEnginePageDescriptors(), getAdvancedEngineStorageMaps(), { nullptr, 0U }, { advancedEngineHooks, _countof(advancedEngineHooks) }, nullptr, nullptr },
  { getTableSwitchingPageDescriptors(), getTableSwitchingStorageMaps(), { nullptr, 0U }, { tableSwitchingHooks, _countof(tableSwitchingHooks) }, hook_apply_table_switching, nullptr },
  { { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { secondarySerialHooks, _countof(secondarySerialHooks) }, nullptr, nullptr },
  { getCommsExtendedPageDescriptors(), getCommsExtendedStorageMaps(), { nullptr, 0U }, { commsExtendedHooks, _countof(commsExtendedHooks) }, nullptr, nullptr },
  { getLoggingPageDescriptors(), getLoggingStorageMaps(), { nullptr, 0U }, { loggingHooks, _countof(loggingHooks) }, nullptr, nullptr },
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

bool core_modules_save_page(uint8_t pageNumber, uint16_t &writesRemaining)
{
  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    const module_page_storage_t &storage = modules[moduleIndex].page_storage;
    for (uint8_t storageIndex = 0; storageIndex < storage.count; ++storageIndex)
    {
      const module_page_storage_descriptor_t &descriptor = storage.descriptors[storageIndex];
      if (descriptor.pageNumber == pageNumber)
      {
        if (descriptor.save != nullptr)
        {
          descriptor.save(writesRemaining);
        }
        return true;
      }
    }
  }

  return false;
}

void core_modules_load_pages(void)
{
  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    const module_page_storage_t &storage = modules[moduleIndex].page_storage;
    for (uint8_t storageIndex = 0; storageIndex < storage.count; ++storageIndex)
    {
      const module_page_storage_descriptor_t &descriptor = storage.descriptors[storageIndex];
      if (descriptor.load != nullptr)
      {
        descriptor.load();
      }
    }
  }
}
