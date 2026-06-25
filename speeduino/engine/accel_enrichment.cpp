#include "engine/accel_enrichment.h"

#include "data/core_constants.h"
#include "data/runtime_state.h"
#include "data/tune_registry.h"
#include "engine/sensors.h"
#include "support/maths.h"
#include "support/preprocessor.h"
#include "support/units.h"
#include "support/unit_testing.h"

static uint16_t aeActivatedReading;
TESTABLE_CONSTEXPR table2D_u8_u8_4 taeTable(&configPage4.taeBins, &configPage4.taeValues);
TESTABLE_CONSTEXPR table2D_u8_u8_4 maeTable(&configPage4.maeBins, &configPage4.maeRates);

static constexpr uint8_t NO_FUEL_CORRECTION = ONE_HUNDRED_PCT;
static constexpr uint8_t BASELINE_FUEL_CORRECTION = ONE_HUNDRED_PCT;

static inline void accelEnrichmentOff(void)
{
  currentStatus.isAcceleratingTPS = false;
  currentStatus.isDeceleratingTPS = false;
  currentStatus.AEamount = 0;
}

static inline bool isAccelEnrichmentOn(void)
{
  return (currentStatus.isAcceleratingTPS) || (currentStatus.isDeceleratingTPS);
}

static inline uint8_t applyAeRpmTaper(uint8_t accelCorrection)
{
  if ((configPage2.aeTaperMax > configPage2.aeTaperMin) && (accelCorrection > 0U))
  {
    const uint16_t taperMinRpm = RPM_COARSE.toUser(configPage2.aeTaperMin);
    if (currentStatus.RPM > taperMinRpm)
    {
      const uint16_t taperMaxRpm = RPM_COARSE.toUser(configPage2.aeTaperMax);
      if (currentStatus.RPM > taperMaxRpm)
      {
        accelCorrection = 0U;
      }
      else
      {
        const auto taperPercent = (uint8_t)map(currentStatus.RPM, taperMinRpm, taperMaxRpm, ONE_HUNDRED_PCT, 0U);
        accelCorrection = (uint8_t)percentageApprox(taperPercent, accelCorrection);
      }
    }
  }
  return accelCorrection;
}

static inline uint16_t applyAeCoolantTaper(uint16_t accelCorrection)
{
  if ((accelCorrection != 0U)
    && (configPage2.aeColdPct != NO_FUEL_CORRECTION)
    && (configPage2.aeColdTaperMax > configPage2.aeColdTaperMin)
    && (currentStatus.coolant < temperatureRemoveOffset(configPage2.aeColdTaperMax)))
  {
    if (currentStatus.coolant <= temperatureRemoveOffset(configPage2.aeColdTaperMin))
    {
      accelCorrection = (uint16_t)percentageApprox(configPage2.aeColdPct, accelCorrection);
    }
    else
    {
      const uint8_t coldPct = BASELINE_FUEL_CORRECTION + map(temperatureAddOffset(currentStatus.coolant),
                                                              configPage2.aeColdTaperMin, configPage2.aeColdTaperMax,
                                                              configPage2.aeColdPct - ONE_HUNDRED_PCT, 0U);
      accelCorrection = (uint16_t)percentageApprox(coldPct, accelCorrection);
    }
  }

  return accelCorrection;
}

static inline uint16_t calcAccelEnrichment(const uint8_t accelCorrection)
{
  currentStatus.isAcceleratingTPS = true;
  return BASELINE_FUEL_CORRECTION + applyAeCoolantTaper(applyAeRpmTaper(accelCorrection));
}

static inline uint16_t calcDeccelEnrichment(void)
{
  currentStatus.isDeceleratingTPS = true;
  return configPage2.decelAmount;
}

static inline bool aeTimeoutExpired(void)
{
  return micros() >= currentStatus.AEEndTime;
}

static inline void updateAeTimeout(void)
{
  currentStatus.AEEndTime = micros() + TIME_TENTH_MILLIS.toUser(configPage2.aeTime);
}

using aeTimeoutExpiredCallback_t = void (*)(void);
using shouldResetCurrentAeCallback_t = bool (*)(void);
using shouldStartAeCallback_t = bool (*)(void);
using computAeCallback_t = uint16_t (*)(void);

static inline uint16_t correctionAccel(const aeTimeoutExpiredCallback_t onTimeoutExpired,
                                       const shouldResetCurrentAeCallback_t shouldResetCurrentAe,
                                       const shouldStartAeCallback_t shouldStartAe,
                                       const computAeCallback_t computeAe)
{
  uint16_t accelCorrection = NO_FUEL_CORRECTION;

  if (isAccelEnrichmentOn())
  {
    if (aeTimeoutExpired())
    {
      accelEnrichmentOff();
      onTimeoutExpired();
    }
    else if (shouldResetCurrentAe())
    {
      accelEnrichmentOff();
    }
    else
    {
      accelCorrection = currentStatus.AEamount;
    }
  }

  if ((!isAccelEnrichmentOn()) && (shouldStartAe()))
  {
    updateAeTimeout();
    accelCorrection = computeAe();
  }

  return accelCorrection;
}

static inline void mapOnTimeoutExpired(void)
{
  currentStatus.mapDOT = 0;
}

static inline bool mapShouldResetAe(void)
{
  return (uint16_t)abs(currentStatus.mapDOT) > aeActivatedReading;
}

static inline bool mapShouldStartAe(void)
{
  return (uint16_t)abs(currentStatus.mapDOT) > configPage2.maeThresh;
}

static inline uint16_t mapComputeAe(void)
{
  uint16_t aeEnrichment = 0U;

  if (currentStatus.mapDOT < 0)
  {
    aeEnrichment = calcDeccelEnrichment();
  }
  else if (currentStatus.mapDOT > 0)
  {
    aeEnrichment = calcAccelEnrichment(table2D_getValue(&maeTable, MAP_DOT.toRaw(currentStatus.mapDOT)));
  }
  else
  {
    ;
  }

  aeActivatedReading = (uint16_t)abs(currentStatus.mapDOT);
  return aeEnrichment;
}

static inline int16_t computeMapDot(void)
{
  int16_t mapDOT = 0U;
  const int16_t mapChange = getMAPDelta();
  if (((uint16_t)abs(mapChange) > configPage2.maeMinChange))
  {
    const uint32_t mapDeltaT = getMAPDeltaTime();
    static constexpr uint32_t MAX_udiv_32_16 = UINT16_MAX;
    static constexpr uint32_t MIN_udiv_32_16 = (MICROS_PER_SEC / UINT16_MAX) + 1U;
    if ((mapDeltaT <= MAX_udiv_32_16) && (mapDeltaT > MIN_udiv_32_16))
    {
      mapDOT = (int16_t)fast_div32_16(MICROS_PER_SEC, mapDeltaT) * (int16_t)mapChange;
    }
    else
    {
      mapDOT = (int16_t)(MICROS_PER_SEC / mapDeltaT) * mapChange;
    }
    static constexpr int16_t MAP_DOT_MIN = -2550;
    static constexpr int16_t MAP_DOT_MAX = 2550;
    mapDOT = constrain(mapDOT, MAP_DOT_MIN, MAP_DOT_MAX);
  }
  return mapDOT;
}

static inline uint16_t correctionAccelModeMap(void)
{
  uint16_t aeCorrection = currentStatus.AEamount;
  if (BIT_CHECK(LOOP_TIMER, MAP_READ_TIMER_BIT))
  {
    currentStatus.mapDOT = computeMapDot();
    aeCorrection = correctionAccel(mapOnTimeoutExpired, mapShouldResetAe, mapShouldStartAe, mapComputeAe);
  }
  return aeCorrection;
}

static inline void tpsOnTimeoutExpired(void)
{
  currentStatus.tpsDOT = 0;
}

static inline bool tpsShouldResetAe(void)
{
  return (uint16_t)abs(currentStatus.tpsDOT) > aeActivatedReading;
}

static inline bool tpsShouldStartAe(void)
{
  return (uint16_t)abs(currentStatus.tpsDOT) > configPage2.taeThresh;
}

static inline uint16_t tpsComputeAe(void)
{
  uint16_t aeEnrichment = 0U;
  if (currentStatus.tpsDOT < 0)
  {
    aeEnrichment = calcDeccelEnrichment();
  }
  else if (currentStatus.tpsDOT > 0)
  {
    aeEnrichment = calcAccelEnrichment(table2D_getValue(&taeTable, TPS_DOT.toRaw(currentStatus.tpsDOT)));
  }
  else
  {
    ;
  }
  aeActivatedReading = (uint16_t)abs(currentStatus.tpsDOT);
  return aeEnrichment;
}

static inline int16_t computeTPSDOT(void)
{
  const int16_t tpsChange = (int16_t)currentStatus.TPS - (int16_t)currentStatus.TPSlast;
  int16_t tpsDOT = 0;
  if ((uint16_t)abs(tpsChange) > configPage2.taeMinChange)
  {
    tpsDOT = (TPS_READ_FREQUENCY * tpsChange) / 2;
    static constexpr int16_t TPS_DOT_MIN = -2550;
    static constexpr int16_t TPS_DOT_MAX = 2550;
    tpsDOT = constrain(tpsDOT, TPS_DOT_MIN, TPS_DOT_MAX);
  }
  return tpsDOT;
}

static inline uint16_t correctionAccelModeTps(void)
{
  uint16_t aeCorrection = currentStatus.AEamount;
  if (BIT_CHECK(LOOP_TIMER, TPS_READ_TIMER_BIT))
  {
    currentStatus.tpsDOT = computeTPSDOT();
    aeCorrection = correctionAccel(tpsOnTimeoutExpired, tpsShouldResetAe, tpsShouldStartAe, tpsComputeAe);
  }
  return aeCorrection;
}

uint16_t correctionAccel(void)
{
  if (AE_MODE_MAP == configPage2.aeMode) { return correctionAccelModeMap(); }
  if (AE_MODE_TPS == configPage2.aeMode) { return correctionAccelModeTps(); }
  return NO_FUEL_CORRECTION;
}
