#ifndef _02D968_STOP_SPAWN_H_
#define _02D968_STOP_SPAWN_H_

/* Course-start setup for the bus-stop passenger subsystem: the six
 * bus-interior anchor points, plus one 02d19c passenger task per already-picked
 * waiting passenger (var_waitingPassengers_8c228798) and per scripted stop-schedule slot
 * (var_stopSchedule_8c228718). Called once by GameEnterDrive_8c01306e. */
void StopSpawnInit_8c02d968(void);

#endif /* _02D968_STOP_SPAWN_H_ */
