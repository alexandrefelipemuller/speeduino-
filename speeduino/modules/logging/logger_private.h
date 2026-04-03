#ifndef LOGGER_PRIVATE_H
#define LOGGER_PRIVATE_H

#include <stdint.h>
#include "statuses.h"
#include "bit_manip.h"

static inline byte setStatusBit(byte status, uint8_t index, bool bit)
{
  BIT_WRITE(status, index, bit);
  return status;
}

static inline byte setStatusBits(byte status, bool (&bits)[1])
{
  return setStatusBit(status, 0U, bits[0U]);
}

template <uint8_t N>
static inline byte setStatusBits(byte status, bool (&bits)[N])
{
  using shorter_t = bool(&)[N-1U];
  return setStatusBits(
    setStatusBit(status, N-1U, bits[N-1U]),
    (shorter_t)bits);
}

static inline byte buildStatus1(const statuses &current)
{
  bool bits[] = {
    current.isInj1Open,
    current.isInj2Open,
    current.isInj3Open,
    current.isInj4Open,
    current.isDFCOActive,
    false,
    current.isToothLog1Full,
  };
  return setStatusBits(0U, bits);
}

static inline byte buildStatus2(const statuses &current)
{
  bool bits[] = {
    current.hardLaunchActive,
    current.softLaunchActive,
    current.hardLimitActive,
    current.softLimitActive,
    false,
    false,
    current.idleOn,
    current.decoder.getStatus().syncStatus == SyncStatus::Full,
  };
  return setStatusBits(0U, bits);
}

static inline byte buildStatus3(const statuses &current)
{
  bool bits[] = {
    current.resetPreventActive,
    current.nitrousActive,
    current.secondFuelTableActive,
    current.vssUiRefresh,
    current.decoder.getStatus().syncStatus == SyncStatus::Partial,
  };
  byte status3 = setStatusBits(0U, bits);
  status3 |= (current.nSquirtsStatus << 5U);
  return status3;
}

static inline byte buildStatus4(const statuses &current)
{
  bool bits[] = {
    current.wmiTankEmpty,
    current.vvt1AngleError,
    current.vvt2AngleError,
    current.fanOn,
    current.burnPending,
    current.stagingActive,
    current.commCompat,
    current.allowLegacyComms,
  };
  return setStatusBits(0U, bits);
}

static inline byte buildStatus5(const statuses &current)
{
  bool bits[] = {
    false,
    current.flatShiftSoftCut,
    current.secondSparkTableActive,
    current.knockRetardActive,
    current.knockPulseDetected,
    current.clutchTriggerActive,
  };
  return setStatusBits(0U, bits);
}

static inline byte buildTestOutput(const statuses &current)
{
  bool bits[] = {
    current.isTestModeActive,
  };
  return setStatusBits(0U, bits);
}

static inline byte buildAirConStatus(const statuses &current)
{
  bool bits[] = {
    current.airconRequested,
    current.airconCompressorOn,
    current.airconRpmLockout,
    current.airconTpsLockout,
    current.airconTurningOn,
    current.airconCltLockout,
    current.airconFanOn,
  };
  return setStatusBits(0U, bits);
}

static inline byte buildEngineProtectStatus(const statuses &current)
{
  bool bits[] = {
    current.engineProtectRpm,
    current.engineProtectBoostCut,
    current.engineProtectOil,
    current.engineProtectAfr,
    current.engineProtectClt,
    false,
    current.engineProtectIoError,
  };
  return setStatusBits(0U, bits);
}

#endif
