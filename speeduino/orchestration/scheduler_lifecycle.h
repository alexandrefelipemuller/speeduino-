/** @file
 * Scheduler lifecycle helpers for fuel/ignition setup and timer control.
 */
#ifndef SCHEDULER_LIFECYCLE_H
#define SCHEDULER_LIFECYCLE_H

void initialiseIgnitionSchedulers(void);
void startIgnitionSchedulers(void);
void stopIgnitionSchedulers(void);
void refreshIgnitionSchedule1(unsigned long timeToEnd);

void initialiseFuelSchedulers(void);
void startFuelSchedulers(void);
void stopFuelSchedulers(void);

#endif // SCHEDULER_LIFECYCLE_H
