// Declaration of Libraries
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Motor
#include "control.h"
#include "sensor.h"
#include <Debounce.h>
#include <AccelStepper.h>

// Encoder
#include <ClickEncoder.h>
// Timer 1 for encoder
#include <TimerOne.h>
// Save Function
#include <EEPROMex.h>
#include <AccelStepper.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
bool refreshScreen = false;

byte enterChar[] = {
    B10000,
    B10000,
    B10100,
    B10110,
    B11111,
    B00110,
    B00100,
    B00000};

byte fastChar[] = {
    B00110,
    B01110,
    B00110,
    B00110,
    B01111,
    B00000,
    B00100,
    B01110};
byte slowChar[] = {
    B00011,
    B00111,
    B00011,
    B11011,
    B11011,
    B00000,
    B00100,
    B01110};

unsigned long previousMillis = 0;
const long interval = 1000;

unsigned long previousMillis2 = 0;
const long interval2 = 200;
unsigned long currentMillis2 = 0;

// Declaration of LCD Variables
const int numOfMainScreens = 3;
const int numOfSettingScreens = 8;
const int numOfTestMenu = 11;
int currentScreen = 0;

int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {
    {"SETTINGS", "ENTER TO EDIT"},
    {"RUN AUTO", "ENTER TO RUN AUTO"},
    {"TEST MACHINE", "ENTER TO TEST"}};

String settings[numOfSettingScreens][2] = {
    {"RICE FLOUR", "MIN"},
    {"LIQUID", "MIN"},
    {"BOILING", "MIN"},
    {"COOKING", "MIN"},
    {"DISPENSE", "MIN"},
    {"CUT INTERVAL", "SEC"},
    {"DROP DELAY", "SEC"},
    {"SAVE SETTINGS", "ENTER TO SAVE"}};

String TestMenuScreen[numOfTestMenu] = {
    "RICE FLOUR",
    "LIQUID",
    "MIXER",
    "HEATER",
    "LINEAR DOOR",
    "EXTRUDER",
    "CUTTER",
    "CONVEYOR 1",
    "CONVEYOR 2",
    "RICE DUSTER",
    "Back to Main Menu"};

double parametersTimer[numOfSettingScreens] = {1, 1, 1, 1, 1, 1, 1};
double parametersTimerMaxValue[numOfSettingScreens] = {1200, 1200, 1200, 1200, 1200, 1200, 1200};

int RiceFlourTimeAdd = 10;
int LiquidTimeAdd = 20;
int BoilingTimeAdd = 30;
int CookingTimeAdd = 40;
int DispenseTimeAdd = 50;
int CutTimeAdd = 60;
int DropTimeAdd = 70;

void saveSettings()
{
    EEPROM.writeDouble(RiceFlourTimeAdd, parametersTimer[0]);
    EEPROM.writeDouble(LiquidTimeAdd, parametersTimer[1]);
    EEPROM.writeDouble(BoilingTimeAdd, parametersTimer[2]);
    EEPROM.writeDouble(CookingTimeAdd, parametersTimer[3]);
    EEPROM.writeDouble(DispenseTimeAdd, parametersTimer[4]);
    EEPROM.writeDouble(CutTimeAdd, parametersTimer[5]);
    EEPROM.writeDouble(DropTimeAdd, parametersTimer[6]);
}

void loadSettings()
{
    parametersTimer[0] = EEPROM.readDouble(RiceFlourTimeAdd);
    parametersTimer[1] = EEPROM.readDouble(LiquidTimeAdd);
    parametersTimer[2] = EEPROM.readDouble(BoilingTimeAdd);
    parametersTimer[3] = EEPROM.readDouble(CookingTimeAdd);
    parametersTimer[4] = EEPROM.readDouble(DispenseTimeAdd);
    parametersTimer[5] = EEPROM.readDouble(CutTimeAdd);
    parametersTimer[6] = EEPROM.readDouble(DropTimeAdd);
}

char *secondsToHHMMSS(int total_seconds)
{
    int hours, minutes, seconds;

    hours = total_seconds / 3600;         // Divide by number of seconds in an hour
    total_seconds = total_seconds % 3600; // Get the remaining seconds
    minutes = total_seconds / 60;         // Divide by number of seconds in a minute
    seconds = total_seconds % 60;         // Get the remaining seconds

    // Format the output string
    static char hhmmss_str[7]; // 6 characters for HHMMSS + 1 for null terminator
    sprintf(hhmmss_str, "%02d%02d%02d", hours, minutes, seconds);
    return hhmmss_str;
}

bool settingsFlag = false;
bool settingEditTimerFlag = false;
bool runAutoFlag = false;
bool testMenuFlag = false;

// Declaration of Variables
// Rotary Encoder Variables
boolean up = false;
boolean down = false;
boolean middle = false;
ClickEncoder *encoder;
int16_t last, value;
// Fast Scroll
bool fastScroll = false;

int ena1 = 22;
int dir1 = 23;
int step1 = 24;

int ena2 = 25;
int dir2 = 26;
int step2 = 27;

int Conveyor1ena = 9;
int Conveyor1dir = 10;
int Conveyor1step = 11;

int ena3 = 28;
int dir3 = 29;
int step3 = 30;

// Control Declaration
AccelStepper RiceFlour(AccelStepper::FULL2WIRE, dir1, step1);
Control RiceFlourTimer(100, 100, 100);
Control Liquid(43, 100, 100);
Control Boiling(100, 100, 100);
Control Mixer(40, 101, 101);
Control Heater(45, 100, 100);
Control Dispensing(100, 100, 100);
Control Linear(42, 100, 100);
Control Extruder(41, 100, 100);
Control CutterDelay(100, 100, 100);
Control CuttingDelay(100, 100, 100);
Control Cutter(44, 100, 100);
Control RiceDuster(47, 100, 100);
Control RiceDusterDelay(100, 100, 100);
AccelStepper stepConveyor1(AccelStepper::FULL2WIRE, dir2, step2);
AccelStepper Conveyor1(AccelStepper::FULL2WIRE, Conveyor1dir, Conveyor1step);
AccelStepper stepConveyor2(AccelStepper::FULL2WIRE, dir3, step3);

Control TimerForNextDrop(100, 100, 100);
const int pinSen1 = A1;
const int pinSen2 = A0;

Debounce Sensor1(pinSen1, 100, true);
Debounce Sensor2(pinSen2, 100, true);

void initSensors()
{
    pinMode(pinSen1, INPUT_PULLUP);
    pinMode(pinSen2, INPUT_PULLUP);
}

long currentPos1 = 0;
long lastPos1 = 0;
long speedStep1 = 8000;
long moveStep1 = 8000;

long currentPos2 = 0;
long lastPos2 = 0;
long speedStep2 = 500;
long moveStep2 = -1000;

long Conveyor1currentPos = 0;
long Conveyor1lastPos = 0;
long Conveyor1speedStep = 400;
long Conveyor1moveStep = -1000;

long currentPos3 = 0;
long lastPos3 = 0;
long speedStep3 = 500;
long moveStep3 = -1000;

void setRiceFlour()
{
    RiceFlour.setEnablePin(ena1);
    RiceFlour.setPinsInverted(false, false, false);
    RiceFlour.setMaxSpeed(speedStep1);
    RiceFlour.setSpeed(speedStep1);
    RiceFlour.setAcceleration(speedStep1 * 200);
    RiceFlour.enableOutputs();
    lastPos1 = RiceFlour.currentPosition();
}

void setStepper1()
{
    stepConveyor1.setEnablePin(ena2);
    stepConveyor1.setPinsInverted(false, false, false);
    stepConveyor1.setMaxSpeed(speedStep2);
    stepConveyor1.setSpeed(speedStep2);
    stepConveyor1.setAcceleration(speedStep2 * 200);
    stepConveyor1.enableOutputs();
    lastPos2 = stepConveyor1.currentPosition();
}

void setConveyor1()
{
    Conveyor1.setEnablePin(Conveyor1ena);
    Conveyor1.setPinsInverted(false, false, false);
    Conveyor1.setMaxSpeed(Conveyor1speedStep);
    Conveyor1.setSpeed(Conveyor1speedStep);
    Conveyor1.setAcceleration(Conveyor1speedStep * 200);
    Conveyor1.enableOutputs();
    lastPos2 = Conveyor1.currentPosition();
}

void setStepper2()
{
    stepConveyor2.setEnablePin(ena3);
    stepConveyor2.setPinsInverted(false, false, false);
    stepConveyor2.setMaxSpeed(speedStep3);
    stepConveyor2.setSpeed(speedStep3);
    stepConveyor2.setAcceleration(speedStep3 * 200);
    stepConveyor2.enableOutputs();
    lastPos3 = stepConveyor2.currentPosition();
}

void DisableSteppers()
{
    RiceFlour.disableOutputs();
    stepConveyor1.disableOutputs();
    stepConveyor2.disableOutputs();
    Conveyor1.disableOutputs();
}
void EnableSteppers()
{
    RiceFlour.enableOutputs();
    stepConveyor1.enableOutputs();
    stepConveyor2.enableOutputs();
    Conveyor1.enableOutputs();
}

bool testFlagRice, testFlagConveyor1, testFlagConveyor2, testFlagCutter = false;

void testRiceFLour()
{
    if (RiceFlour.distanceToGo() == 0)
    {
        RiceFlour.setCurrentPosition(0);
        RiceFlour.move(moveStep1);
    }
}

void testConveyor1()
{
    if (stepConveyor1.distanceToGo() == 0)
    {
        stepConveyor1.setCurrentPosition(0);
        stepConveyor1.move(moveStep2);
    }
}

void testRunConveyor1()
{
    if (Conveyor1.distanceToGo() == 0)
    {
        Conveyor1.setCurrentPosition(0);
        Conveyor1.move(Conveyor1moveStep);
    }
}

void testConveyor2()
{
    if (stepConveyor2.distanceToGo() == 0)
    {
        stepConveyor2.setCurrentPosition(0);
        stepConveyor2.move(moveStep3);
    }
}

void testRun()
{
    if (testFlagRice == true)
    {
        RiceFlour.enableOutputs();
        testRiceFLour();
    }
    else
    {
        RiceFlour.disableOutputs();
    }

    if (testFlagConveyor1 == true)
    {
        Conveyor1.enableOutputs();
        testRunConveyor1();
    }
    else
    {
        Conveyor1.disableOutputs();
    }

    if (testFlagConveyor2 == true)
    {
        stepConveyor2.enableOutputs();
        testConveyor2();
    }
    else
    {
        stepConveyor2.disableOutputs();
    }

    if (testFlagCutter == true)
    {
        RunCut();
    }
}

int cutStat = 0;
void RunCut()
{
    switch (cutStat)
    {
    case 0:
        CutterDelay.run();
        if (CutterDelay.isTimerCompleted() == true)
        {
            currentMillis2 = millis();
            previousMillis2 = currentMillis2;
            cutStat = 1;
        }
        break;
    case 1:
        cutterCutRun();
        break;
    default:
        break;
    }
}

void cutterCutRun()
{
    currentMillis2 = millis();
    if (currentMillis2 - previousMillis2 >= interval2)
    {
        previousMillis2 = currentMillis2;
        Cutter.relayOff();
        CutterDelay.start();
        cutStat = 0;
    }
    else
    {
        Cutter.relayOn();
    }
}

void setTimers()
{
    RiceFlourTimer.setTimer(secondsToHHMMSS(parametersTimer[0] * 60));
    Liquid.setTimer(secondsToHHMMSS(parametersTimer[1] * 60));
    Boiling.setTimer(secondsToHHMMSS(parametersTimer[2] * 60));
    Heater.setTimer(secondsToHHMMSS(parametersTimer[3] * 60));
    Dispensing.setTimer(secondsToHHMMSS(parametersTimer[4] * 60));
    CutterDelay.setTimer(secondsToHHMMSS(parametersTimer[5]));
    CuttingDelay.setTimer(secondsToHHMMSS(30));
    TimerForNextDrop.setTimer(secondsToHHMMSS(parametersTimer[6]));
}

void RunRiceFlour()
{
    if (RiceFlour.distanceToGo() == 0)
    {
        RiceFlour.setCurrentPosition(0);
        RiceFlour.move(moveStep1);
    }
}

void RunConveyor1()
{
    if (stepConveyor1.distanceToGo() == 0)
    {
        stepConveyor1.setCurrentPosition(0);
        stepConveyor1.move(moveStep2);
    }
}

void RunConveyor1upd()
{
    if (Conveyor1.distanceToGo() == 0)
    {
        Conveyor1.setCurrentPosition(0);
        Conveyor1.move(Conveyor1moveStep);
    }
}

void RunConveyor2()
{
    if (stepConveyor2.distanceToGo() == 0)
    {
        stepConveyor2.setCurrentPosition(0);
        stepConveyor2.move(moveStep3);
    }
}

void runAllConveyor()
{
    RiceFlour.run();
    stepConveyor1.run();
    stepConveyor2.run();
    Conveyor1.run();
}

void stopAll()
{
    RiceFlourTimer.relayOff();
    RiceFlourTimer.stop();

    Liquid.relayOff();
    Liquid.stop();

    Boiling.relayOff();
    Boiling.stop();

    Mixer.relayOff();
    Mixer.stop();

    Heater.relayOff();
    Heater.stop();

    Linear.relayOff();
    Linear.stop();

    Extruder.relayOff();
    Extruder.stop();

    CutterDelay.relayOff();
    CutterDelay.stop();

    CuttingDelay.relayOff();
    CuttingDelay.stop();

    Cutter.relayOff();
    Cutter.stop();

    RiceDuster.relayOff();
    RiceDuster.stop();

    RiceDusterDelay.relayOff();
    RiceDusterDelay.stop();

    TimerForNextDrop.relayOff();
    TimerForNextDrop.stop();

    DisableSteppers();
}

int RunAutoFlag = 0;
int DispenseFlag = 0;
bool initialMoveConveyorLow = false;
int runPackageStat = 0;
/*
1 - Dispense
2 - Cooking
3 - Extruding
*/

/*
1 - Dispense Liquid
2 - Boil
3 - Dispense Dry Ingre
4 - Cooking
5 - Extruding
*/

void RunAuto()
{
    switch (RunAutoFlag)
    {
    case 1:
        RunDispense();
        break;
    case 2:
        RunCooking();
        break;
    case 3:
        RunDispenseFromMixer();
        break;
    case 4:
        Extruding();
        break;
    default:
        stopAll();
        RunAutoFlag = 0;
        break;
    }
}

void RunDispense()
{
    switch (DispenseFlag)
    {
    case 0:
        Liquid.run();
        Heater.relayOn();
        Mixer.relayOn();
        if (Liquid.isTimerCompleted() == true)
        {
            DispenseFlag = 1;
            Boiling.start();
        }
        break;
    case 1:
        Boiling.run();
        if (Boiling.isTimerCompleted() == true)
        {
            DispenseFlag = 2;
            RiceFlourTimer.start();
        }
        break;
    case 2:
        RiceFlour.enableOutputs();
        RiceFlour.run();
        RiceFlourTimer.run();
        if (RiceFlourTimer.isTimerCompleted() == true)
        {
            DispenseFlag = 0;
            RunAutoFlag = 2;
            Heater.start();
            RiceFlour.disableOutputs();
        }
        else
        {
            RunRiceFlour();
        }
        break;
    default:
        break;
    }
}

void RunCooking()
{
    Heater.run();
    Mixer.relayOn();
    if (Heater.isTimerCompleted() == true)
    {
        Dispensing.start();
        RunAutoFlag = 3;
    }
}

void RunDispenseFromMixer()
{
    Dispensing.run();
    Mixer.relayOn();
    Linear.relayOn();
    if (Dispensing.isTimerCompleted() == true)
    {
        Linear.relayOff();
        Mixer.relayOff();
        RunAutoFlag = 4;
        runPackageStat = 1;
        setConveyor1();
        Conveyor1.enableOutputs();
        stepConveyor2.enableOutputs();
        CuttingDelay.start();
    }
}

bool cutRunFlag = false;

void CutterStartDelay()
{
    CuttingDelay.run();
    if (CuttingDelay.isTimerCompleted() == true)
    {
        cutRunFlag = true;
        CutterDelay.start();
        cutStat = 0;
    }
}

void Extruding()
{
    if (cutRunFlag == false)
    {
        CutterStartDelay();
    }
    else
    {
        RunCut();
        RiceDuster.relayOn();
    }
    Extruder.relayOn();
    Conveyor1.run();
    testRunConveyor1();
    readSensors();
    runPackaging();
}

bool Sensor1Stat,Sensor2Stat = false;
void readSensors()
{
    Sensor1Stat = Sensor1.read();
    Sensor2Stat = Sensor2.read();
}

void runPackaging()
{
    switch (runPackageStat)
    {
    case 1:
        if (initialMoveConveyorLow == true)
        {
            if (Sensor2Stat == 1)
            {
                stepConveyor2.run();
                RunConveyor2();
            }
            else
            {
                initialMoveConveyorLow = false;
            }
        }
        else
        {
            if (Sensor2Stat == 0)
            {
                stepConveyor2.run();
                RunConveyor2();
            }
            else
            {
                stepConveyor2.disableOutputs();
                runPackageStat = 2;
            }
        }
        break;
    case 2:
        if (Sensor1Stat == 0)
        {
            TimerForNextDrop.start();
            runPackageStat = 3;
        }
        break;
    case 3:
        TimerForNextDrop.run();
        if (TimerForNextDrop.isTimerCompleted() == true)
        {
            runPackageStat = 1;
            stepConveyor2.enableOutputs();
            initialMoveConveyorLow = true;
        }
        break;

    default:
        break;
    }
}

// Functions for Rotary Encoder
void timerIsr()
{
    encoder->service();
}

void readRotaryEncoder()
{
    value += encoder->getValue();

    if (value / 2 > last)
    {
        last = value / 2;
        down = true;
        delay(100);
    }
    else if (value / 2 < last)
    {
        last = value / 2;
        up = true;
        delay(100);
    }
}

void readButtonEncoder()
{
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open)
    { // Open Bracket for Click
        switch (b)
        { // Open Bracket for Double Click
        case ClickEncoder::Clicked:
            middle = true;
            break;

        case ClickEncoder::DoubleClicked:
            refreshScreen = 1;
            if (settingsFlag)
            {
                if (fastScroll == false)
                {
                    fastScroll = true;
                }
                else
                {
                    fastScroll = false;
                }
            }
            break;
        }
    }
}

void inputCommands()
{
    // LCD Change Function and Values
    //  To Right Rotary
    if (up == 1)
    {
        up = false;
        refreshScreen = true;
        if (settingsFlag == true)
        {
            if (settingEditTimerFlag == true)
            {
                if (parametersTimer[currentSettingScreen] >= parametersTimerMaxValue[currentSettingScreen] - 1)
                {
                    parametersTimer[currentSettingScreen] = parametersTimerMaxValue[currentSettingScreen];
                }
                else
                {
                    if (fastScroll == true)
                    {
                        parametersTimer[currentSettingScreen] += 1;
                    }
                    else
                    {
                        parametersTimer[currentSettingScreen] += 0.1;
                    }
                }
            }
            else
            {
                if (currentSettingScreen == numOfSettingScreens - 1)
                {
                    currentSettingScreen = 0;
                }
                else
                {
                    currentSettingScreen++;
                }
            }
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == numOfTestMenu - 1)
            {
                currentTestMenuScreen = 0;
            }
            else
            {
                currentTestMenuScreen++;
            }
        }
        else
        {
            if (currentScreen == numOfMainScreens - 1)
            {
                currentScreen = 0;
            }
            else
            {
                currentScreen++;
            }
        }
    }

    // To Left Rotary
    if (down == 1)
    {
        down = false;
        refreshScreen = true;
        if (settingsFlag == true)
        {
            if (settingEditTimerFlag == true)
            {
                if (parametersTimer[currentSettingScreen] <= 0)
                {
                    parametersTimer[currentSettingScreen] = 0;
                }
                else
                {
                    if (fastScroll == true)
                    {
                        parametersTimer[currentSettingScreen] -= 1;
                    }
                    else
                    {
                        parametersTimer[currentSettingScreen] -= 0.1;
                    }
                }
            }
            else
            {
                if (currentSettingScreen <= 0)
                {
                    currentSettingScreen = numOfSettingScreens - 1;
                }
                else
                {
                    currentSettingScreen--;
                }
            }
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen <= 0)
            {
                currentTestMenuScreen = numOfTestMenu - 1;
            }
            else
            {
                currentTestMenuScreen--;
            }
        }
        else
        {
            if (currentScreen == 0)
            {
                currentScreen = numOfMainScreens - 1;
            }
            else
            {
                currentScreen--;
            }
        }
    }

    // Rotary Button Press
    if (middle == 1)
    {
        middle = false;
        refreshScreen = 1;
        if (currentScreen == 0 && settingsFlag == true)
        {
            if (currentSettingScreen == numOfSettingScreens - 1)
            {
                settingsFlag = false;
                saveSettings();
                loadSettings();
                setTimers();
                currentSettingScreen = 0;
            }
            else
            {
                if (settingEditTimerFlag == true)
                {
                    settingEditTimerFlag = false;
                }
                else
                {
                    settingEditTimerFlag = true;
                }
            }
        }
        else if (runAutoFlag == true)
        {
            runAutoFlag = false;
            stopAll();
            RunAutoFlag = 0;
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == numOfTestMenu - 1)
            {
                currentTestMenuScreen = 0;
                testMenuFlag = false;
            }
            else if (currentTestMenuScreen == 0)
            {
                if (testFlagRice == false)
                {
                    testFlagRice = true;
                }
                else
                {
                    testFlagRice = false;
                }
            }
            else if (currentTestMenuScreen == 1)
            {
                if (Liquid.getMotorState() == false)
                {
                    Liquid.relayOn();
                }
                else
                {
                    Liquid.relayOff();
                }
            }
            else if (currentTestMenuScreen == 2)
            {
                if (Mixer.getMotorState() == false)
                {
                    Mixer.relayOn();
                }
                else
                {
                    Mixer.relayOff();
                }
            }
            else if (currentTestMenuScreen == 3)
            {
                if (Heater.getMotorState() == false)
                {
                    Heater.relayOn();
                }
                else
                {
                    Heater.relayOff();
                }
            }
            else if (currentTestMenuScreen == 4)
            {
                if (Linear.getMotorState() == false)
                {
                    Linear.relayOn();
                }
                else
                {
                    Linear.relayOff();
                }
            }
            else if (currentTestMenuScreen == 5)
            {
                if (Extruder.getMotorState() == false)
                {
                    Extruder.relayOn();
                }
                else
                {
                    Extruder.relayOff();
                }
            }
            else if (currentTestMenuScreen == 6)
            {
                if (testFlagCutter == false)
                {
                    testFlagCutter = true;
                    cutStat = 0;
                    CutterDelay.start();
                }
                else
                {
                    testFlagCutter = false;
                    CutterDelay.stop();
                }
            }
            else if (currentTestMenuScreen == 7)
            {
                if (testFlagConveyor1 == false)
                {
                    testFlagConveyor1 = true;
                }
                else
                {
                    testFlagConveyor1 = false;
                }
            }
            else if (currentTestMenuScreen == 8)
            {
                if (testFlagConveyor2 == false)
                {
                    testFlagConveyor2 = true;
                }
                else
                {
                    testFlagConveyor2 = false;
                }
            }
            else if (currentTestMenuScreen == 9)
            {
                if (RiceDuster.getMotorState() == false)
                {
                    RiceDuster.relayOn();
                }
                else
                {
                    RiceDuster.relayOff();
                }
            }
        }
        else
        {
            if (currentScreen == 0)
            {
                settingsFlag = true;
            }
            else if (currentScreen == 1)
            {
                runAutoFlag = true;
                // Insert Commands for Run Auto
                RunAutoFlag = 1;
                RiceFlourTimer.start();
                Liquid.start();
                refreshScreen = 1;
            }
            else if (currentScreen == 2)
            {
                testMenuFlag = true;
            }
        }
    }
}

void PrintRunAuto(String job, String secondJob, char *time)
{
    lcd.clear();
    lcd.print("Running Auto");
    lcd.setCursor(0, 1);
    lcd.print("Status: " + job);
    lcd.setCursor(0, 2);
    lcd.print(secondJob);
    lcd.setCursor(0, 3);
    lcd.print("Timer: ");
    lcd.setCursor(7, 3);
    lcd.print(time);
}

void printScreen()
{

    if (settingsFlag == true)
    {
        lcd.clear();
        lcd.print(settings[currentSettingScreen][0]);
        lcd.setCursor(0, 1);
        if (currentSettingScreen == numOfSettingScreens - 1)
        {
            lcd.setCursor(0, 3);
            lcd.write(0);
            lcd.setCursor(2, 3);
            lcd.print("Click to Save All");
        }
        else
        {
            lcd.setCursor(0, 1);
            lcd.print(parametersTimer[currentSettingScreen]);
            lcd.print(" ");
            lcd.print(settings[currentSettingScreen][1]);
            lcd.setCursor(0, 3);
            lcd.write(0);
            lcd.setCursor(2, 3);
            if (settingEditTimerFlag == false)
            {
                lcd.print("ENTER TO EDIT");
            }
            else
            {
                lcd.print("ENTER TO SAVE");
            }
            lcd.setCursor(19, 3);
            if (fastScroll == true)
            {
                lcd.write(1);
            }
            else
            {
                lcd.write(2);
            }
        }
    }
    else if (runAutoFlag == true)
    {
        switch (RunAutoFlag)
        {
        case 1:
            switch (DispenseFlag)
            {
            case 0:
                PrintRunAuto("Liquid Ingre", "", Liquid.getTimeRemaining());
                break;
            case 1:
                PrintRunAuto("Boil", "", Boiling.getTimeRemaining());
                break;
            case 2:
                PrintRunAuto("Dry Ingre", "", RiceFlourTimer.getTimeRemaining());
                break;
            default:
                break;
            }
            break;
        case 2:
            PrintRunAuto("Cooking", "", Heater.getTimeRemaining());
            break;
        case 3:
            PrintRunAuto("Dispensing", "", Dispensing.getTimeRemaining());
            break;
        case 4:
            switch (runPackageStat)
            {
            case 1:
                char buffer[10];
                PrintRunAuto("Extruding", "Waiting for Tray", itoa(Conveyor1.currentPosition(),buffer,0));
                break;
            case 2:
                PrintRunAuto("Extruding", "Waiting for Drop", TimerForNextDrop.getTimeRemaining());
                break;
            case 3:
                PrintRunAuto("Extruding", "Dropping", TimerForNextDrop.getTimeRemaining());
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    else if (testMenuFlag == true)
    {
        lcd.clear();
        lcd.print(TestMenuScreen[currentTestMenuScreen]);

        if (currentTestMenuScreen == 0)
        {
            if (testFlagRice == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("DRY INGRE : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("DRY INGRE : OFF");
            }
        }
        else if (currentTestMenuScreen == 1)
        {
            if (Liquid.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("LIQUID INGRE : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("DRY INGRE : OFF");
            }
        }
        else if (currentTestMenuScreen == 2)
        {
            if (Mixer.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("MIXER : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("MIXER : OFF");
            }
        }
        else if (currentTestMenuScreen == 3)
        {
            if (Heater.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("HEATER : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("HEATER : OFF");
            }
        }
        else if (currentTestMenuScreen == 4)
        {
            if (Linear.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("DOOR : OPEN");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("DOOR : CLOSE");
            }
        }
        else if (currentTestMenuScreen == 5)
        {
            if (Extruder.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("EXTRUDER : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("EXTRUDER : OFF");
            }
        }
        else if (currentTestMenuScreen == 6)
        {
            if (testFlagCutter == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("CUTTER : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("CUTTER : OFF");
            }
        }
        else if (currentTestMenuScreen == 7)
        {
            if (testFlagConveyor1 == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("CONVE1 : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("CONVE1 : OFF");
            }
        }
        else if (currentTestMenuScreen == 8)
        {
            if (testFlagConveyor2 == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("CONVE2 : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("CONVE2 : OFF");
            }
        }
        else if (currentTestMenuScreen == 9)
        {
            if (RiceDuster.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("RICE DUSTER : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("RICE DUSTER : OFF");
            }
        }

        if (currentTestMenuScreen == numOfTestMenu - 1)
        {
            lcd.setCursor(0, 3);
            lcd.print("Click to Exit Test");
        }
        else
        {
            lcd.setCursor(0, 3);
            lcd.print("Click to Run Test");
        }
    }
    else
    {
        lcd.clear();
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0, 3);
        lcd.write(0);
        lcd.setCursor(2, 3);
        lcd.print(screens[currentScreen][1]);
        refreshScreen = false;
    }
}

void setupJumper()
{
}

void setup()
{
    encoder = new ClickEncoder(3, 2, 4); // Actual
    // encoder = new ClickEncoder(3, 4, 2); // TestBench
    encoder->setAccelerationEnabled(false);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
    last = encoder->getValue();

    // LCD Setup
    lcd.init();
    lcd.createChar(0, enterChar);
    lcd.createChar(1, fastChar);
    lcd.createChar(2, slowChar);
    lcd.clear();
    lcd.backlight();
    refreshScreen = true;
    Serial.begin(9600);
    setRiceFlour();
    setStepper1();
    setStepper2();
    setConveyor1();
    // saveSettings(); // Disable upon Initialiaze
    loadSettings();
    setTimers();
    initSensors();
    DisableSteppers();
}

void loop()
{
    readRotaryEncoder();
    readButtonEncoder();
    inputCommands();

    if (refreshScreen == true)
    {
        printScreen();
        refreshScreen = false;
    }

    if (testMenuFlag == true)
    {
        testRun();
    }

    if (runAutoFlag == true)
    {
        RunAuto();
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            // save the last time you blinked the LED
            previousMillis = currentMillis;
            refreshScreen = true;
        }
    }

    if (runAutoFlag == false)
    {
        runAllConveyor();
    }
}