#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "constants.h"
#include "thresholds.h"
#include <Arduino.h>


class StateMachine
{
public:
    StateMachine();
    void begin();
    void update(float relativeNoiseLevel);
    void requestState(DisplayState requested);
    void enterConfigMode();
    void exitConfigMode(bool save = false);
    void cycleConfigMode();
    void acknowledgeAlarm();
    void showCalibrationProgress(float progress, int frame, int secondsElapsed);
    DisplayState getCurrentState() const;
    bool isAlarmActive() const;
    bool isInConfigMode() const;
    int getActiveModeId() const;
    float getCurrentThreshold() const;

private:
    DisplayState currentState;
    DisplayState lastState;
    bool alarmActive;
    bool inConfigMode;
    bool saving;
    unsigned long alarmTriggeredAt;
    unsigned long alarmAcknowledgedAt;  // Track when alarm was last acknowledged
    unsigned long configModeStartTime;
    int activeModeId;
    float currentThreshold;
    void updateAlarmLogic(float relativeNoiseLevel);
    void handleConfigMode();
};

#endif // STATE_MACHINE_H
