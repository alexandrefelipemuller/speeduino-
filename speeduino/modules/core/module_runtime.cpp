#include "module_runtime.h"

#include "module_registry.h"

namespace {
static bool registry_is_valid(void)
{
  static const bool valid = core_modules_validate_registry();
  return valid;
}

static void run_phase(module_hook_phase_t phase, module_runtime_context_t &context)
{
  if (!registry_is_valid())
  {
    return;
  }

  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    const module_descriptor_t &module = modules[moduleIndex];
    for (uint8_t hookIndex = 0; hookIndex < module.hooks.count; ++hookIndex)
    {
      const module_hook_descriptor_t &hookDescriptor = module.hooks.hooks[hookIndex];
      if (hookDescriptor.phase == phase)
      {
        hookDescriptor.hook(context);
      }
    }
  }
}
} // namespace

void core_modules_init_pre_pin_mapping(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::init_pre_pin_mapping, context);
}

void core_modules_init_post_pin_mapping(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::init_post_pin_mapping, context);
}

void core_modules_poll(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::poll, context);
}

void core_modules_on_engine_stop(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::on_engine_stop, context);
}

void core_modules_tick_50hz(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::tick_50hz, context);
}

void core_modules_tick_30hz(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::tick_30hz, context);
}

void core_modules_tick_15hz(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::tick_15hz, context);
}

void core_modules_tick_10hz(void)
{
  module_runtime_context_t context;
  run_phase(module_hook_phase_t::tick_10hz, context);
}

void core_modules_tick_4hz(uint8_t sensor_status, statuses &current)
{
  module_runtime_context_t context;
  context.sensor_status = sensor_status;
  context.current_status = &current;
  run_phase(module_hook_phase_t::tick_4hz, context);
}

void core_modules_tick_1hz(const statuses &current)
{
  module_runtime_context_t context;
  context.current_status_const = &current;
  run_phase(module_hook_phase_t::tick_1hz, context);
}

void core_modules_apply_table_switching(statuses &current)
{
  if (!registry_is_valid())
  {
    return;
  }

  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    if (modules[moduleIndex].apply_table_switching != nullptr)
    {
      modules[moduleIndex].apply_table_switching(current);
    }
  }
}

statuses::scheduler_cut_t core_modules_get_scheduler_cut(statuses &current)
{
  if (!registry_is_valid())
  {
    return current.schedulerCutState;
  }

  const module_descriptor_t *modules = getRegisteredModules();
  const uint8_t moduleCount = getRegisteredModuleCount();
  statuses::scheduler_cut_t result = current.schedulerCutState;

  for (uint8_t moduleIndex = 0; moduleIndex < moduleCount; ++moduleIndex)
  {
    if (modules[moduleIndex].get_scheduler_cut != nullptr)
    {
      result = modules[moduleIndex].get_scheduler_cut(current);
    }
  }

  return result;
}
