#include "modules/advanced_engine/nitrous.h"

#include "data/advanced_engine_status.h"
#include "support/atomic.h"
#include "boards/board_definition.h"
#include "support/port_pin.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "support/units.h"
#include "support/utilities.h"

static port_register_t n2o_arming_pin_port;
static pin_mask_t n2o_arming_pin_mask;

static inline uint8_t getN2oArmPinPolarity(const config10 &page10)
{
  if(page10.n2o_pin_polarity == 1U)
  {
    return INPUT_PULLUP;
  }
  return INPUT;
}

static void initialiseN2oArmPin(const config10 &page10)
{
  if(configPage10.n2o_enable != 0U && !pinIsReserved(page10.n2o_arming_pin))
  {
    pinMode(page10.n2o_arming_pin, getN2oArmPinPolarity(page10));
    n2o_arming_pin_port = portInputRegister(digitalPinToPort(page10.n2o_arming_pin));
    n2o_arming_pin_mask = digitalPinToBitMask(page10.n2o_arming_pin);
  }
}

#if defined(CORE_TEENSY) || defined(CORE_STM32)

#define N2O_STAGE1_PIN_LOW()    (digitalWrite(configPage10.n2o_stage1_pin, LOW))
#define N2O_STAGE1_PIN_HIGH()   (digitalWrite(configPage10.n2o_stage1_pin, HIGH))
#define N2O_STAGE2_PIN_LOW()    (digitalWrite(configPage10.n2o_stage2_pin, LOW))
#define N2O_STAGE2_PIN_HIGH()   (digitalWrite(configPage10.n2o_stage2_pin, HIGH))

static void initialiseN2oPins(const config10 &page10)
{
  pinMode(page10.n2o_stage1_pin, OUTPUT);
  pinMode(page10.n2o_stage2_pin, OUTPUT);
  initialiseN2oArmPin(page10);
}

#else

static port_register_t n2o_stage1_pin_port;
static pin_mask_t n2o_stage1_pin_mask;
static port_register_t n2o_stage2_pin_port;
static pin_mask_t n2o_stage2_pin_mask;

#define N2O_STAGE1_PIN_LOW()    ATOMIC() { *n2o_stage1_pin_port &= ~(n2o_stage1_pin_mask);  }
#define N2O_STAGE1_PIN_HIGH()   ATOMIC() { *n2o_stage1_pin_port |= (n2o_stage1_pin_mask);   }
#define N2O_STAGE2_PIN_LOW()    ATOMIC() { *n2o_stage2_pin_port &= ~(n2o_stage2_pin_mask);  }
#define N2O_STAGE2_PIN_HIGH()   ATOMIC() { *n2o_stage2_pin_port |= (n2o_stage2_pin_mask);   }

static void initialiseN2oPins(const config10 &page10)
{
  pinMode(page10.n2o_stage1_pin, OUTPUT);
  n2o_stage1_pin_port = portOutputRegister(digitalPinToPort(page10.n2o_stage1_pin));
  n2o_stage1_pin_mask = digitalPinToBitMask(page10.n2o_stage1_pin);
  pinMode(page10.n2o_stage2_pin, OUTPUT);
  n2o_stage2_pin_port = portOutputRegister(digitalPinToPort(page10.n2o_stage2_pin));
  n2o_stage2_pin_mask = digitalPinToBitMask(page10.n2o_stage2_pin);
  initialiseN2oArmPin(page10);
}

#endif

#define READ_N2O_ARM_PIN()    ((*n2o_arming_pin_port & n2o_arming_pin_mask) ? true : false)

void initialiseNitrous(const config10 &page10)
{
  initialiseN2oPins(page10);

  // 255 means uninitialised board data; keep legacy safety behavior.
  if(configPage10.n2o_minTPS == 255) { configPage10.n2o_enable = 0; }

  currentAdvancedEngineStatus.nitrous_status = NITROUS_OFF;
}

void nitrousControl(void)
{
  currentAdvancedEngineStatus.nitrous_active = false;
  currentAdvancedEngineStatus.nitrous_status = NITROUS_OFF;

  if(configPage10.n2o_enable > 0)
  {
    bool isArmed = READ_N2O_ARM_PIN();
    if(configPage10.n2o_pin_polarity == 1) { isArmed = !isArmed; }

    if((isArmed == true) &&
       (currentStatus.coolant > temperatureRemoveOffset(configPage10.n2o_minCLT)) &&
       (currentStatus.TPS > configPage10.n2o_minTPS) &&
       (currentStatus.O2 < configPage10.n2o_maxAFR) &&
       (currentStatus.MAP < (uint16_t)(configPage10.n2o_maxMAP * 2)))
    {
      const uint16_t realStage1MinRPM = (uint16_t)configPage10.n2o_stage1_minRPM * 100;
      const uint16_t realStage1MaxRPM = (uint16_t)configPage10.n2o_stage1_maxRPM * 100;
      const uint16_t realStage2MinRPM = (uint16_t)configPage10.n2o_stage2_minRPM * 100;
      const uint16_t realStage2MaxRPM = (uint16_t)configPage10.n2o_stage2_maxRPM * 100;

      if((currentStatus.RPM > realStage1MinRPM) && (currentStatus.RPM < realStage1MaxRPM))
      {
        currentAdvancedEngineStatus.nitrous_status += NITROUS_STAGE1;
        currentAdvancedEngineStatus.nitrous_active = true;
        N2O_STAGE1_PIN_HIGH();
      }
      if(configPage10.n2o_enable == NITROUS_STAGE2)
      {
        if((currentStatus.RPM > realStage2MinRPM) && (currentStatus.RPM < realStage2MaxRPM))
        {
          currentAdvancedEngineStatus.nitrous_status += NITROUS_STAGE2;
          currentAdvancedEngineStatus.nitrous_active = true;
          N2O_STAGE2_PIN_HIGH();
        }
      }
    }
  }

  if(currentAdvancedEngineStatus.nitrous_active == false)
  {
    if(configPage10.n2o_enable > 0)
    {
      N2O_STAGE1_PIN_LOW();
      N2O_STAGE2_PIN_LOW();
    }
  }
}
