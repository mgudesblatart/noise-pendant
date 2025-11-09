#include "state_machine.h"
#include "nvs_config.h"

const char *displayStateNames[] = {
    "OFF", "BAR_GRAPH", "ALARM", "CONFIG", "SAVING", "CALIBRATION"};

StateMachine::StateMachine()
    : currentState(DISPLAY_OFF), lastState(DISPLAY_OFF), alarmActive(false), inConfigMode(false),
      alarmTriggeredAt(0), alarmAcknowledgedAt(0), configModeStartTime(0), activeModeId(0) {}

void StateMachine::begin()
{

    CurrentModeData currentModeData = loadCurrentMode();
    activeModeId = currentModeData.modeId;
    currentState = DISPLAY_OFF;
    lastState = DISPLAY_OFF;
    alarmActive = false;
    inConfigMode = false;
    alarmTriggeredAt = 0;
    configModeStartTime = 0;
    saving = false;
    currentThreshold = currentModeData.currentThreshold;
}

void StateMachine::update(float relativeNoiseLevel)
{
    if (inConfigMode)
    {
        handleConfigMode();
        return;
    }
    updateAlarmLogic(relativeNoiseLevel);
}

void StateMachine::requestState(DisplayState requested)
{
    // Serial.printf("StateMachine: Requesting state %s Current State: %s Current Mode: %s\n", displayStateNames[requested], displayStateNames[currentState], modeInfos[activeModeId].name);
    // Priority: CONFIG > ALARM > SAVING > BAR_GRAPH > OFF
    switch (requested)
    {
    case DISPLAY_CONFIG:
        inConfigMode = true;
        configModeStartTime = millis();
        currentState = DISPLAY_CONFIG;
        break;
    case DISPLAY_ALARM:
        alarmActive = true;
        currentState = DISPLAY_ALARM;
        alarmTriggeredAt = millis();
        break;
    case DISPLAY_SAVING:
        currentState = DISPLAY_SAVING;
        break;
    case DISPLAY_BAR_GRAPH:
        if (!alarmActive && !inConfigMode)
            currentState = DISPLAY_BAR_GRAPH;
        break;
    case DISPLAY_OFF:
        if (!alarmActive && !inConfigMode)
            currentState = DISPLAY_OFF;
        break;
    case DISPLAY_CALIBRATION:
        currentState = DISPLAY_CALIBRATION;
        break;
    default:
        break;
    }
}

void StateMachine::updateAlarmLogic(float relativeNoiseLevel)
{
    lastState = currentState;

    // Check if we're in cooldown period after acknowledgment (5 seconds)
    bool inCooldown = (alarmAcknowledgedAt > 0) && (millis() - alarmAcknowledgedAt < 5000);

    if (alarmActive || (relativeNoiseLevel >= 100.0f && !inCooldown))
    {
        if (!alarmActive && relativeNoiseLevel >= 100.0f && !inCooldown)
        {
            alarmActive = true;
            alarmTriggeredAt = millis();
        }
        requestState(DISPLAY_ALARM);
    }
    else if (relativeNoiseLevel >= 25.0f)
    {
        alarmActive = false;
        requestState(DISPLAY_BAR_GRAPH);
    }
    else
    {
        alarmActive = false;
        requestState(DISPLAY_OFF);
    }
}

void StateMachine::acknowledgeAlarm()
{
    if (alarmActive)
    {
        alarmActive = false;
        alarmAcknowledgedAt = millis();  // Record acknowledgment time for cooldown
        requestState(DISPLAY_BAR_GRAPH);
        Serial.println("Alarm acknowledged - 5 second cooldown active");
    }
}

void StateMachine::enterConfigMode()
{
    requestState(DISPLAY_CONFIG);
}

void StateMachine::exitConfigMode(bool save)
{
    if (save)
    {
        saving = true;
        requestState(DISPLAY_SAVING);
        saveCurrentMode(activeModeId);
        delay(3000);
        inConfigMode = false;
        saving = false;
        return;
    }
    activeModeId = loadCurrentMode().modeId;
    inConfigMode = false;
}

void StateMachine::cycleConfigMode()
{
    activeModeId = (activeModeId + 1) % NUM_MODES;
    requestState(DISPLAY_CONFIG);
    configModeStartTime = millis();
}

void StateMachine::handleConfigMode()
{
    if (!inConfigMode || saving)
        return;
    if (millis() - configModeStartTime > CONFIG_MODE_TIMEOUT_MS)
    {
        exitConfigMode(false);
    }
}

DisplayState StateMachine::getCurrentState() const
{
    return currentState;
}

bool StateMachine::isAlarmActive() const
{
    return alarmActive;
}

bool StateMachine::isInConfigMode() const
{
    return inConfigMode;
}

int StateMachine::getActiveModeId() const
{
    return activeModeId;
}

float StateMachine::getCurrentThreshold() const
{
    return currentThreshold;
}