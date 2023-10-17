// Declaration of Libraries
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// Motor
#include "control.h"
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
const int numOfSettingScreens = 10;
const int numOfTestMenu = 12;
int currentScreen = 0;

int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {
    {"SETTINGS", "ENTER TO EDIT"},
    {"RUN AUTO", "ENTER TO RUN AUTO"},
    {"TEST MACHINE", "ENTER TO TEST"}};

String settings[numOfSettingScreens][2] = {
    {"SUGAR", "MIN"},
    {"COOKING 1", "MIN"},
    {"PUMP", "MIN"},
    {"COOKING 2", "MIN"},
    {"MANI", "MIN"},
    {"COOKING 3", "MIN"},
    {"DISPENSE TO EXTRUDE", "MIN"},
    {"EXTRUDE", "SEC"},
    {"PRESS", "SEC"},
    {"SAVE SETTINGS", "ENTER TO SAVE"}};

String TestMenuScreen[numOfTestMenu] = {
    "MANI",
    "SUGAR",
    "PUMP",
    "MIXER",
    "HEATER MIXER",
    "LINEAR",
    "HEATER BARREL",
    "EXTRUDER",
    "EXTRUDER GATE",
    "CONVEYOR",
    "FLATTENER"
    "BACK TO MAIN MENU"};

double parametersTimer[numOfSettingScreens] = {1, 1, 1, 1, 1, 1, 1,1,1,1};
double parametersTimerMaxValue[numOfSettingScreens] = {1200, 1200, 1200, 1200, 1200, 1200, 1200,1200,1200,1200};

int SugarTimeAdd = 10;
int Cooking1TimeAdd = 20;
int PumpTimeAdd = 30;
int Cooking2TimeAdd = 40;
int ManiTimeAdd = 50;
int Cooking3TimeAdd = 60;
int DispenseToExtrudeTimeAdd = 70;
int ExtrudeTimeAdd = 80;
int PressTimeAdd = 90;

void saveSettings()
{
    EEPROM.writeDouble(SugarTimeAdd, parametersTimer[0]);
    EEPROM.writeDouble(Cooking1TimeAdd, parametersTimer[1]);
    EEPROM.writeDouble(PumpTimeAdd, parametersTimer[2]);
    EEPROM.writeDouble(Cooking2TimeAdd, parametersTimer[3]);
    EEPROM.writeDouble(ManiTimeAdd, parametersTimer[4]);
    EEPROM.writeDouble(Cooking3TimeAdd, parametersTimer[5]);
    EEPROM.writeDouble(DispenseToExtrudeTimeAdd, parametersTimer[6]);
    EEPROM.writeDouble(ExtrudeTimeAdd, parametersTimer[7]);
    EEPROM.writeDouble(PressTimeAdd, parametersTimer[8]);
}

void loadSettings()
{
    parametersTimer[0] = EEPROM.readDouble(SugarTimeAdd);
    parametersTimer[1] = EEPROM.readDouble(Cooking1TimeAdd);
    parametersTimer[2] = EEPROM.readDouble(PumpTimeAdd);
    parametersTimer[3] = EEPROM.readDouble(Cooking2TimeAdd);
    parametersTimer[4] = EEPROM.readDouble(ManiTimeAdd);
    parametersTimer[5] = EEPROM.readDouble(Cooking3TimeAdd);
    parametersTimer[6] = EEPROM.readDouble(DispenseToExtrudeTimeAdd);
    parametersTimer[7] = EEPROM.readDouble(ExtrudeTimeAdd);
    parametersTimer[8] = EEPROM.readDouble(PressTimeAdd);
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

int ena1 = 28;
int dir1 = 29;
int step1 = 30;

int ena2 = 9;
int dir2 = 10;
int step2 = 11;

// Control Declaration
// AccelStepper RiceFlour(AccelStepper::FULL2WIRE, dir1, step1);
// Control RiceFlourTimer(100, 100, 100);
// Control Liquid(43, 100, 100);
// Control Boiling(100, 100, 100);
// Control Mixer(40, 101, 101);
// Control Heater(45, 100, 100);
// Control Dispensing(100, 100, 100);
// Control Linear(42, 100, 100);
// Control Extruder(41, 100, 100);
// Control CutterDelay(100, 100, 100);
// Control CuttingDelay(100, 100, 100);
// Control Cutter(44, 100, 100);
// Control RiceDuster(47, 100, 100);
// Control RiceDusterDelay(100, 100, 100);
// AccelStepper stepConveyor1(AccelStepper::FULL2WIRE, dir2, step2);
// AccelStepper Conveyor1(AccelStepper::FULL2WIRE, Conveyor1dir, Conveyor1step);
// AccelStepper stepConveyor2(AccelStepper::FULL2WIRE, dir3, step3);

AccelStepper Sugar(AccelStepper::FULL2WIRE, dir1, step1);
Control Water(43, 100, 100);
Control Mixer(40, 101, 101);
Control Mani(46, 100, 100);
Control Heater1(45, 100, 100);
Control Linear(42, 100, 100);
Control Heater2(38, 100, 100);
Control Extruder(41, 100, 100);
Control GateExtruder(47, 100, 100);
Control FlatGate(44, 100, 100);
AccelStepper stepConveyor(AccelStepper::FULL2WIRE, dir2, step2);



// Utility Timer
Control SugarTimer(100, 100, 100);
Control CookingTimer(100, 100, 100);
Control Cooking2Timer(100, 100, 100);
Control Cooking3Timer(100, 100, 100);


const int pinSen1 = A1;
const int pinSen2 = A0;
bool SenStat1, SenStat2 = false;

Debounce Sensor1(pinSen1, 100, true);
Debounce Sensor2(pinSen2, 100, true);

void ReadSensor()
{
    SenStat1 = Sensor1.read();
    SenStat2 = Sensor2.read();
}

void initSensors()
{
    pinMode(pinSen1, INPUT_PULLUP);
    pinMode(pinSen2, INPUT_PULLUP);
}

long currentPos1 = 0;
long lastPos1 = 0;
long speedStep1 = 8000;
long moveStep1 = 10000;

long currentPos2 = 0;
long lastPos2 = 0;
long speedStep2 = 2000;
long moveStep2 = 2000;

void setSugarStepper()
{
    Sugar.setEnablePin(ena1);
    Sugar.setPinsInverted(false, false, false);
    Sugar.setMaxSpeed(speedStep1);
    Sugar.setSpeed(speedStep1);
    Sugar.setAcceleration(speedStep1 * 200);
    Sugar.enableOutputs();
    lastPos2 = Sugar.currentPosition();
}

void setConveyorStepper()
{
    stepConveyor.setEnablePin(ena2);
    stepConveyor.setPinsInverted(false, false, false);
    stepConveyor.setMaxSpeed(speedStep2);
    stepConveyor.setSpeed(speedStep2);
    stepConveyor.setAcceleration(speedStep2 * 200);
    stepConveyor.enableOutputs();
    lastPos2 = stepConveyor.currentPosition();
}

void DisableSteppers()
{
    stepConveyor.disableOutputs();
    Sugar.disableOutputs();
}
void EnableSteppers()
{
    stepConveyor.enableOutputs();
    Sugar.enableOutputs();
}

bool testFlagSugar, testFlagConveyor1 = false;

void runSugarStepper()
{
    if (Sugar.distanceToGo() == 0)
    {
        Sugar.setCurrentPosition(0);
        Sugar.move(moveStep1);
    }
}

void runConveyorStepper()
{
    if (stepConveyor.distanceToGo() == 0)
    {
        stepConveyor.setCurrentPosition(0);
        stepConveyor.move(moveStep2);
    }
}

void testRun()
{
    if (testFlagSugar == true)
    {
        Sugar.enableOutputs();
        runSugarStepper();
        Sugar.run();
    }
    else
    {
        Sugar.disableOutputs();
    }

    if (testFlagConveyor1 == true)
    {
        stepConveyor.enableOutputs();
        runConveyorStepper();
        stepConveyor.run();
    }
    else
    {
        stepConveyor.disableOutputs();
    }
}

void RunSteppers(){
    Sugar.run();
    stepConveyor.run();
}

void setTimers()
{
    SugarTimer.setTimer(secondsToHHMMSS(parametersTimer[0] * 60));
    CookingTimer.setTimer(secondsToHHMMSS(parametersTimer[1] * 60));
    Water.setTimer(secondsToHHMMSS(parametersTimer[2] * 60));
    Cooking2Timer.setTimer(secondsToHHMMSS(parametersTimer[3] * 60));
    Mani.setTimer(secondsToHHMMSS(parametersTimer[4] * 60));
    Cooking3Timer.setTimer(secondsToHHMMSS(parametersTimer[5] * 60));
    Linear.setTimer(secondsToHHMMSS(parametersTimer[6] * 60));
    Extruder.setTimer(secondsToHHMMSS(parametersTimer[7]));
    FlatGate.setTimer(secondsToHHMMSS(parametersTimer[8]));
}

void runAllConveyor()
{
    Sugar.run();
    stepConveyor.run();
}

void stopAll()
{
    DisableSteppers();
    Water.stop();
    Water.relayOff();

    Mixer.stop();
    Mixer.relayOff();

    Mani.stop();
    Mani.relayOff();

    Heater1.stop();
    Heater1.relayOff();

    Linear.stop();
    Linear.relayOff();

    Heater2.relayOff();

    Extruder.stop();
    Extruder.relayOff();

    GateExtruder.stop();
    GateExtruder.relayOff();

    FlatGate.stop();
    FlatGate.relayOff();

    SugarTimer.stop();
    SugarTimer.relayOff();

    CookingTimer.stop();
    CookingTimer.relayOff();

    Cooking2Timer.stop();
    Cooking2Timer.relayOff();

    Cooking3Timer.stop();
    Cooking3Timer.relayOff();
}

int RunAutoFlag = 0;
int DispenseFlag = 0;
bool initialMoveConveyorLow = false;
int runPackageStat = 0;
int extrudeStatus = 0;
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
        DispenseAndCaramelize();
        break;
    case 2:
        DispenseAndAddWater();
        break;
    case 3:
        DispenseAndAddPeanut();
        break;
    case 4:
        CookingRun();
        break;
    case 5:
        DispenseAndPreheatBarrel();
        break;
    case 6:
        RunExtrude();
        break;
    default:
        stopAll();
        RunAutoFlag = 0;
        break;
    }
}

void DispenseAndCaramelize(){
    SugarTimer.run();
    CookingTimer.run();
    Heater1.relayOn();
    Mixer.relayOn();
    Sugar.run();
    if (SugarTimer.isTimerCompleted() == true)
    {

    }
    else
    {
        runSugarStepper();
    }

    if (CookingTimer.isTimerCompleted() == true && SugarTimer.isTimerCompleted() == true)
    {
        RunAutoFlag = 2;
        Water.start();
        Cooking2Timer.start();
        Sugar.disableOutputs();
    }
    
}

void DispenseAndAddWater(){
    Water.run();
    Cooking2Timer.run();
    Heater1.relayOn();
    Mixer.relayOn();
    if (Cooking2Timer.isTimerCompleted() == true && Water.isTimerCompleted() == true)
    {
        RunAutoFlag = 3;
        Mani.start();
    }
}

void DispenseAndAddPeanut(){
    Mani.run();
    Heater1.relayOn();
    Mixer.relayOn();
    if (Mani.isTimerCompleted() == true)
    {
        RunAutoFlag = 4;
        Cooking3Timer.start();
    }
}
bool initialMoveExtruder = false;

void CookingRun(){
    Cooking3Timer.run();
    Heater1.relayOn();
    Heater2.relayOn();
    Mixer.relayOn();
    if (Cooking3Timer.isTimerCompleted() == true)
    {
        Linear.start();
        RunAutoFlag = 6;
        stepConveyor.enableOutputs();
        extrudeStatus = 1;
        initialMoveExtruder = true;
    }
}


void DispenseAndPreheatBarrel(){
    Linear.run();
    Heater1.relayOn();
    Heater2.relayOn();
    Mixer.relayOn();
    if (Linear.isTimerCompleted() == true)
    {
        Mixer.relayOff();
        Heater1.relayOff();
    }
}


void RunExtrude()
{
    if(Linear.isTimerCompleted() == false){
        DispenseAndPreheatBarrel();
    }
    
    ReadSensor();
    switch (extrudeStatus)
    {
    case 1:
        if (initialMoveExtruder == true)
        {
            if (SenStat1 == false)
            {
                runConveyorStepper();
            }
            else
            {
                initialMoveExtruder = false;
            }
        }
        else
        {
            if (SenStat1 == true)
            {
                stepConveyor.disableOutputs();
                Extruder.start();
                extrudeStatus = 2;
            }
            else
            {
                runConveyorStepper();
            }
        }
        break;
    case 2:
        Extruder.run();
        GateExtruder.relayOn();
        if (Extruder.isTimerCompleted() == true)
        {
            GateExtruder.relayOff();
            extrudeStatus = 3;
            initialMoveExtruder = true;
            stepConveyor.enableOutputs();
            delay(1000);
        }
        break;
    case 3:
        
            if (initialMoveExtruder == true)
        {
            if (SenStat2 == false)
            {
                runConveyorStepper();
            }
            else
            {
                initialMoveExtruder = false;
            }
        }
        else
        {
            if (SenStat2 == true)
            {
                stepConveyor.disableOutputs();
                FlatGate.start();
                extrudeStatus = 4;
                delay(1000);
            }
            else
            {
                runConveyorStepper();
            }
        }
        
        break;
    case 4:
        FlatGate.run();
        if (FlatGate.isTimerCompleted() == true)
        {
            extrudeStatus = 1;
            stepConveyor.enableOutputs();
            initialMoveExtruder = true;
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
                if (Mani.getMotorState() == false)
                {
                    Mani.relayOn();
                }
                else
                {
                    Mani.relayOff();
                }
            }
            else if (currentTestMenuScreen == 1)
            {
                if (testFlagSugar == false)
                {
                    testFlagSugar = true;
                }
                else
                {
                    testFlagSugar = false;
                }
            }
            else if (currentTestMenuScreen == 2)
            {
                if (Water.getMotorState() == false)
                {
                    Water.relayOn();
                }
                else
                {
                    Water.relayOff();
                }
            }
            else if (currentTestMenuScreen == 3)
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
            else if (currentTestMenuScreen == 4)
            {
                if (Heater1.getMotorState() == false)
                {
                    Heater1.relayOn();
                }
                else
                {
                    Heater1.relayOff();
                }
            }
            else if (currentTestMenuScreen == 5)
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
            else if (currentTestMenuScreen == 6)
            {
                if (Heater2.getMotorState() == false)
                {
                    Heater2.relayOn();
                }
                else
                {
                    Heater2.relayOff();
                }
            }
            else if (currentTestMenuScreen == 7)
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
            else if (currentTestMenuScreen == 8)
            {
                if (GateExtruder.getMotorState() == false)
                {
                    GateExtruder.relayOn();
                }
                else
                {
                    GateExtruder.relayOff();
                }
            }
            else if (currentTestMenuScreen == 9)
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
            else if (currentTestMenuScreen == 10)
            {
                if (FlatGate.getMotorState() == false)
                {
                    FlatGate.relayOn();
                }
                else
                {
                    FlatGate.relayOff();
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
                refreshScreen = 1;
                SugarTimer.start();
                CookingTimer.start();
                EnableSteppers();
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
                PrintRunAuto("CARAMELIZE","SUGAR",CookingTimer.getTimeRemaining());
                break;
            case 2:
                PrintRunAuto("DEGLAZE", "WATER",Cooking2Timer.getTimeRemaining());
                break;
            case 3:
                PrintRunAuto("PEANUT", "DISPENSE",Mani.getTimeRemaining());
                break;
            case 4:
                PrintRunAuto("COOKING", "MIXING",Cooking3Timer.getTimeRemaining());
                break;
            case 5:
                PrintRunAuto("PRE EXTRUDE", "DISPENSE",Linear.getTimeRemaining());
                break;
            case 6:
                switch (extrudeStatus)
                {
                case 1:
                    PrintRunAuto("CONVEYOR", "TO EXTRUDER","N/A");
                    break;
                case 2:
                    PrintRunAuto("EXTRUDING", "GATE OPEN",Extruder.getTimeRemaining());
                    break;    
                case 3:
                    PrintRunAuto("CONVEYOR", "TO FLATTENER","N/A");
                    break;
                case 4:
                    PrintRunAuto("FLATTENING", "",FlatGate.getTimeRemaining());
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
            if (Mani.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("PEANUT : OPEN");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("PEANUT : CLOSE");
            }
        }
        else if (currentTestMenuScreen == 1)
        {
            if (testFlagSugar == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("SUGAR : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("SUGAR : OFF");
            }
        }
        else if (currentTestMenuScreen == 2)
        {
            if (Water.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("PUMP : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("PUMP : OFF");
            }
        }
        else if (currentTestMenuScreen == 3)
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
        else if (currentTestMenuScreen == 4)
        {
            if (Heater1.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("HEATER MIXER: ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("HEATER MIXER: OFF");
            }
        }
        else if (currentTestMenuScreen == 5)
        {
            if (Linear.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("LINEAR : OPEN");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("LINEAR : CLOSE");
            }
        }
        else if (currentTestMenuScreen == 6)
        {
            if (Heater2.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("HEATER BARREL: ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("HEATER BARREL: OFF");
            }
        }
        else if (currentTestMenuScreen == 7)
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
        else if (currentTestMenuScreen == 8)
        {
            if (GateExtruder.getMotorState() == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("GATE EXTRUDER : OPEN");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("GATE EXTRUDER : CLOSE");
            }
        }
        else if (currentTestMenuScreen == 9)
        {
            if (testFlagConveyor1 == true)
            {
                lcd.setCursor(0, 2);
                lcd.print("CONVEYOR : ON");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("CONVEYOR : OFF");
            }
        }
        else if (currentTestMenuScreen == 10)
        {
            if (FlatGate.getMotorState() == false)
            {
                lcd.setCursor(0, 2);
                lcd.print("FLAT GATE : DOWN");
            }
            else
            {
                lcd.setCursor(0, 2);
                lcd.print("FLAT GATE : UP");
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
    // setRiceFlour();
    // setStepper1();
    // setStepper2();
    // setConveyor1();
    setSugarStepper();
    setConveyorStepper();
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
       // runAllConveyor();
    }

    if (runAutoFlag == true)
    {
        RunAuto();
        RunSteppers();
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