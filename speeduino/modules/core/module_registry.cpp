#include "module_registry.h"

#include "support/preprocessor.h"
#include "data/config9_domains.h"
#include "data/pin_registry.h"
#include "module_interfaces.h"
#include "modules/can/module_can.h"
#include "modules/boost/boost.h"
#include "modules/engine_protection/engine_protection.h"
#include "modules/etb/etb.h"
#include "modules/etb/etb_storage.h"
#include "modules/fan_aircon/module_fan_aircon.h"
#include "modules/launch_flatshift/launch_flatshift.h"
#include "modules/launch_control/launch_control.h"
#include "modules/knock/module_knock.h"
#include "modules/nitrous/module_nitrous.h"
#include "modules/programmable_io/programmable_io.h"
#include "modules/sd_logging/module_sd_logging.h"
#include "modules/vvt/vvt.h"
#include "modules/wmi/wmi.h"
#include "modules/wmi/wmi_storage.h"
#include "data/table_registry.h"
#include "data/tune_registry.h"

namespace {
constexpr module_capability_t coreProvides[] = { module_capability_t::logging };
constexpr module_capability_t sdLoggingProvides[] = { module_capability_t::sd_logging };
constexpr module_capability_t secondarySerialProvides[] = { module_capability_t::secondary_serial };
constexpr module_capability_t commsExtendedProvides[] = { module_capability_t::comms_extended };
constexpr module_capability_t canProvides[] = { module_capability_t::can };
constexpr module_capability_t boostProvides[] = { module_capability_t::boost };
constexpr module_capability_t knockProvides[] = { module_capability_t::knock };
constexpr module_capability_t vvtProvides[] = { module_capability_t::vvt };
constexpr module_capability_t etbProvides[] = { module_capability_t::etb };
constexpr module_capability_t engineProtectionProvides[] = { module_capability_t::engine_protection };
constexpr module_capability_t launchFlatShiftProvides[] = { module_capability_t::launch_flatshift };
constexpr module_capability_t launchControlProvides[] = { module_capability_t::launch, module_capability_t::launch_control };
constexpr module_capability_t fanAirconProvides[] = { module_capability_t::fan_aircon };
constexpr module_capability_t programmableIoProvides[] = { module_capability_t::programmable_io };
constexpr module_capability_t nitrousProvides[] = { module_capability_t::nitrous };
constexpr module_capability_t wmiProvides[] = { module_capability_t::wmi };
constexpr module_capability_t advancedEngineProvides[] = { module_capability_t::advanced_engine };
constexpr module_capability_t tableSwitchingProvides[] = { module_capability_t::table_switching };

constexpr module_capability_t sdLoggingRequires[] = { module_capability_t::logging };
constexpr module_capability_t launchFlatShiftRequires[] = { module_capability_t::launch };

static bool contains_capability(const module_capability_list_t &list, module_capability_t capability)
{
  for (uint8_t index = 0; index < list.count; ++index)
  {
    if (list.capabilities[index] == capability)
    {
      return true;
    }
  }

  return false;
}

static bool validate_registry(void)
{
  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    const module_descriptor_t &module = modules[moduleIndex];
    for (uint8_t requireIndex = 0; requireIndex < module.requires.count; ++requireIndex)
    {
      const module_capability_t requiredCapability = module.requires.capabilities[requireIndex];
      bool satisfied = false;

      for (uint8_t providerIndex = 0; providerIndex < moduleCount; ++providerIndex)
      {
        if (contains_capability(modules[providerIndex].provides, requiredCapability))
        {
          satisfied = true;
          break;
        }
      }

      if (!satisfied)
      {
        return false;
      }
    }
  }

  return true;
}

static const module_capability_list_t coreProvidesList = { coreProvides, _countof(coreProvides) };
static const module_capability_list_t sdLoggingProvidesList = { sdLoggingProvides, _countof(sdLoggingProvides) };
static const module_capability_list_t secondarySerialProvidesList = { secondarySerialProvides, _countof(secondarySerialProvides) };
static const module_capability_list_t commsExtendedProvidesList = { commsExtendedProvides, _countof(commsExtendedProvides) };
static const module_capability_list_t canProvidesList = { canProvides, _countof(canProvides) };
static const module_capability_list_t boostProvidesList = { boostProvides, _countof(boostProvides) };
static const module_capability_list_t knockProvidesList = { knockProvides, _countof(knockProvides) };
static const module_capability_list_t vvtProvidesList = { vvtProvides, _countof(vvtProvides) };
static const module_capability_list_t etbProvidesList = { etbProvides, _countof(etbProvides) };
static const module_capability_list_t engineProtectionProvidesList = { engineProtectionProvides, _countof(engineProtectionProvides) };
static const module_capability_list_t launchFlatShiftProvidesList = { launchFlatShiftProvides, _countof(launchFlatShiftProvides) };
static const module_capability_list_t launchControlProvidesList = { launchControlProvides, _countof(launchControlProvides) };
static const module_capability_list_t fanAirconProvidesList = { fanAirconProvides, _countof(fanAirconProvides) };
static const module_capability_list_t programmableIoProvidesList = { programmableIoProvides, _countof(programmableIoProvides) };
static const module_capability_list_t nitrousProvidesList = { nitrousProvides, _countof(nitrousProvides) };
static const module_capability_list_t wmiProvidesList = { wmiProvides, _countof(wmiProvides) };
static const module_capability_list_t advancedEngineProvidesList = { advancedEngineProvides, _countof(advancedEngineProvides) };
static const module_capability_list_t tableSwitchingProvidesList = { tableSwitchingProvides, _countof(tableSwitchingProvides) };
static const module_capability_list_t emptyCapabilityList = { nullptr, 0U };
static const module_capability_list_t sdLoggingRequiresList = { sdLoggingRequires, _countof(sdLoggingRequires) };
static const module_capability_list_t launchFlatShiftRequiresList = { launchFlatShiftRequires, _countof(launchFlatShiftRequires) };

static void hook_logging_init_pre_pin_mapping(module_runtime_context_t &)
{
  module_sd_logging_init(configPage13);
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

static void hook_can_init_post_pin_mapping(module_runtime_context_t &)
{
  module_can_init();
}

static void hook_secondary_serial_poll(module_runtime_context_t &)
{
  module_secondary_serial_poll(get_secondary_serial_config(configPage9));
}

static void hook_comms_extended_poll(module_runtime_context_t &)
{
  module_comms_extended_poll(configPage9.enable_intcan, configPage2.canWBO);
}

static void hook_can_poll(module_runtime_context_t &)
{
  module_can_poll(configPage9.enable_intcan, configPage2.canWBO);
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

static void hook_knock_init_post_pin_mapping(module_runtime_context_t &)
{
  module_knock_init_post_pin_mapping();
}

static void hook_vvt_on_engine_stop(module_runtime_context_t &)
{
  vvt1Off();
  vvt2Off();
  DISABLE_VVT_TIMER();
}

static void hook_etb_init_post_pin_mapping(module_runtime_context_t &)
{
  module_etb_init_post_pin_mapping();
}

static void hook_etb_tick_200hz(module_runtime_context_t &)
{
  module_etb_tick_200hz();
}

static void hook_etb_on_engine_stop(module_runtime_context_t &)
{
  module_etb_on_engine_stop();
}

static void hook_comms_extended_tick_50hz(module_runtime_context_t &)
{
  module_comms_extended_tick_50hz();
}

static void hook_can_tick_50hz(module_runtime_context_t &)
{
  module_can_tick_50hz();
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

static void hook_can_tick_30hz(module_runtime_context_t &)
{
  module_can_tick_30hz();
}

static void hook_logging_tick_30hz(module_runtime_context_t &)
{
  module_sd_logging_tick_30hz(configPage13);
}

static void hook_comms_extended_tick_15hz(module_runtime_context_t &)
{
  module_comms_extended_tick_15hz();
}

static void hook_can_tick_15hz(module_runtime_context_t &)
{
  module_can_tick_15hz();
}

static void hook_launch_flatshift_tick_15hz(module_runtime_context_t &)
{
  module_launch_flatshift_tick_15hz();
}

static void hook_launch_control_init_post_pin_mapping(module_runtime_context_t &)
{
  module_launch_control_init_post_pin_mapping();
}

static void hook_launch_control_tick_10hz(module_runtime_context_t &)
{
  module_launch_control_tick_10hz();
}

static void hook_launch_control_on_engine_stop(module_runtime_context_t &)
{
  module_launch_control_on_engine_stop();
}

static void hook_nitrous_tick_4hz(module_runtime_context_t &)
{
  module_nitrous_tick_4hz();
}

static void hook_knock_tick_10hz(module_runtime_context_t &)
{
  module_knock_tick_10hz();
}

static void hook_knock_on_engine_stop(module_runtime_context_t &)
{
  module_knock_on_engine_stop();
}

static void hook_comms_extended_tick_10hz(module_runtime_context_t &)
{
  module_comms_extended_tick_10hz();
}

static void hook_can_tick_10hz(module_runtime_context_t &)
{
  module_can_tick_10hz();
}

static void hook_advanced_engine_tick_10hz(module_runtime_context_t &)
{
  module_advanced_engine_tick_10hz();
}

static void hook_logging_tick_10hz(module_runtime_context_t &)
{
  module_sd_logging_tick_10hz(configPage13);
}

static void hook_comms_extended_tick_4hz(module_runtime_context_t &context)
{
  module_comms_extended_tick_4hz(context.sensor_status, *context.current_status, get_can_extended_config(configPage9));
}

static void hook_can_tick_4hz(module_runtime_context_t &context)
{
  module_can_tick_4hz(context.sensor_status, *context.current_status, get_can_extended_config(configPage9));
}

static void hook_logging_tick_4hz(module_runtime_context_t &)
{
  module_sd_logging_tick_4hz(configPage13);
}

static void hook_logging_tick_1hz(module_runtime_context_t &context)
{
  module_sd_logging_tick_1hz(*context.current_status_const, configPage13);
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

static constexpr module_hook_descriptor_t etbHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_etb_init_post_pin_mapping },
  { module_hook_phase_t::tick_200hz, hook_etb_tick_200hz },
  { module_hook_phase_t::on_engine_stop, hook_etb_on_engine_stop },
};

static constexpr module_page_storage_descriptor_t etbPageStorage[] = {
  { etbPage, module_etb_save_pages, module_etb_load_pages },
};

static constexpr module_hook_descriptor_t engineProtectionHooks[] = {};

static constexpr module_hook_descriptor_t launchFlatShiftHooks[] = {
  { module_hook_phase_t::tick_15hz, hook_launch_flatshift_tick_15hz },
};

static constexpr module_hook_descriptor_t launchControlHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_launch_control_init_post_pin_mapping },
  { module_hook_phase_t::on_engine_stop, hook_launch_control_on_engine_stop },
  { module_hook_phase_t::tick_10hz, hook_launch_control_tick_10hz },
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

static constexpr module_hook_descriptor_t knockHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_knock_init_post_pin_mapping },
  { module_hook_phase_t::tick_10hz, hook_knock_tick_10hz },
  { module_hook_phase_t::on_engine_stop, hook_knock_on_engine_stop },
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

static constexpr module_hook_descriptor_t canHooks[] = {
  { module_hook_phase_t::init_post_pin_mapping, hook_can_init_post_pin_mapping },
  { module_hook_phase_t::poll, hook_can_poll },
  { module_hook_phase_t::tick_50hz, hook_can_tick_50hz },
  { module_hook_phase_t::tick_30hz, hook_can_tick_30hz },
  { module_hook_phase_t::tick_15hz, hook_can_tick_15hz },
  { module_hook_phase_t::tick_10hz, hook_can_tick_10hz },
  { module_hook_phase_t::tick_4hz, hook_can_tick_4hz },
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
  { coreProvidesList, emptyCapabilityList, getCorePageDescriptors(), getCoreStorageMaps(), { nullptr, 0U }, { coreHooks, _countof(coreHooks) }, nullptr, nullptr },
  { boostProvidesList, emptyCapabilityList, getBoostPageDescriptors(), getBoostStorageMaps(), { boostPageStorage, _countof(boostPageStorage) }, { boostHooks, _countof(boostHooks) }, nullptr, nullptr },
  { vvtProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { vvtHooks, _countof(vvtHooks) }, nullptr, nullptr },
  { etbProvidesList, emptyCapabilityList, getEtbPageDescriptors(), getEtbStorageMaps(), { etbPageStorage, _countof(etbPageStorage) }, { etbHooks, _countof(etbHooks) }, nullptr, nullptr },
  { engineProtectionProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { engineProtectionHooks, _countof(engineProtectionHooks) }, nullptr, hook_engine_protection_scheduler_cut },
  { launchFlatShiftProvidesList, launchFlatShiftRequiresList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { launchFlatShiftHooks, _countof(launchFlatShiftHooks) }, nullptr, nullptr },
  { launchControlProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { launchControlHooks, _countof(launchControlHooks) }, nullptr, nullptr },
  { fanAirconProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { fanAirconHooks, _countof(fanAirconHooks) }, nullptr, nullptr },
  { programmableIoProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { programmableIoHooks, _countof(programmableIoHooks) }, nullptr, nullptr },
  { nitrousProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { nitrousHooks, _countof(nitrousHooks) }, nullptr, nullptr },
  { knockProvidesList, emptyCapabilityList, getKnockPageDescriptors(), getKnockStorageMaps(), { nullptr, 0U }, { knockHooks, _countof(knockHooks) }, nullptr, nullptr },
  { wmiProvidesList, emptyCapabilityList, getWmiPageDescriptors(), getWmiStorageMaps(), { wmiPageStorage, _countof(wmiPageStorage) }, { wmiHooks, _countof(wmiHooks) }, nullptr, nullptr },
  { advancedEngineProvidesList, emptyCapabilityList, getAdvancedEnginePageDescriptors(), getAdvancedEngineStorageMaps(), { nullptr, 0U }, { advancedEngineHooks, _countof(advancedEngineHooks) }, nullptr, nullptr },
  { tableSwitchingProvidesList, emptyCapabilityList, getTableSwitchingPageDescriptors(), getTableSwitchingStorageMaps(), { nullptr, 0U }, { tableSwitchingHooks, _countof(tableSwitchingHooks) }, hook_apply_table_switching, nullptr },
  { secondarySerialProvidesList, emptyCapabilityList, { nullptr, 0U }, { nullptr, 0U }, { nullptr, 0U }, { secondarySerialHooks, _countof(secondarySerialHooks) }, nullptr, nullptr },
  { canProvidesList, emptyCapabilityList, getCanPageDescriptors(), getCanStorageMaps(), { nullptr, 0U }, { canHooks, _countof(canHooks) }, nullptr, nullptr },
  { commsExtendedProvidesList, emptyCapabilityList, getCommsExtendedPageDescriptors(), getCommsExtendedStorageMaps(), { nullptr, 0U }, { commsExtendedHooks, _countof(commsExtendedHooks) }, nullptr, nullptr },
  { sdLoggingProvidesList, sdLoggingRequiresList, getSdLoggingPageDescriptors(), getSdLoggingStorageMaps(), { nullptr, 0U }, { loggingHooks, _countof(loggingHooks) }, nullptr, nullptr },
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

bool core_modules_validate_registry(void)
{
  return validate_registry();
}

bool core_modules_save_page(uint8_t pageNumber, uint16_t &writesRemaining)
{
  if (!validate_registry())
  {
    return false;
  }

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
  if (!validate_registry())
  {
    return;
  }

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
