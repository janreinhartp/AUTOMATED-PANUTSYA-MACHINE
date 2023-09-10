// Declaration of Libraries
// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
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

// Declaration of LCD Variables
const int numOfMainScreens = 3;
const int numOfSettingScreens = 5;
const int numOfTestMenu = 7;
int currentScreen = 0;

int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {
    {"SETTINGS", "ENTER TO EDIT"},
    {"RUN AUTO", "ENTER TO RUN AUTO"},
    {"TEST MACHINE", "ENTER TO TEST"}};

String settings[numOfSettingScreens][2] = {
    {"GRINDING", "MIN"},
    {"MIXING", "MIN"},
    {"SLICE", "SEC"},
    {"SLICEINTERVAL", "SEC"},
    {"SAVE SETTINGS", "ENTER TO SAVE"}};

String TestMenuScreen[numOfTestMenu] = {
    "GRINDER",
    "MIXING",
    "EXTRUDING",
    "CUTTER",
    "CHEESE GRATER",
    "CONVEYOR",
    "Back to Main Menu"};

double parametersTimer[numOfSettingScreens] = {1, 1, 1, 1, 1};
double parametersTimerMaxValue[numOfSettingScreens] = {120, 120, 120, 120, 120};

bool settingsFlag = false;
bool settingEditTimerFlag = false;
bool runAutoFlag = false;
bool testMenuFlag = false;

// Motor
#include "control.h"

// Encoder
#include <ClickEncoder.h>
// Timer 1 for encoder
#include <TimerOne.h>
// Save Function
#include <EEPROMex.h>
#include <AccelStepper.h>

// Declaration of Variables
// Rotary Encoder Variables
boolean up = false;
boolean down = false;
boolean middle = false;
ClickEncoder *encoder;
int16_t last, value;
// Fast Scroll
bool fastScroll = false;

unsigned long previousMillis = 0; // will store last time LED was updated
// constants won't change:
const long interval = 1000; // interval at which to blink (milliseconds)

unsigned long previousMillis2 = 0; // will store last time LED was updated
// constants won't change:
const long interval2 = 200; // interval at which to blink (milliseconds)
unsigned long currentMillis2 = 0;

// MOTOR DECLARATION
Control Grinder(40, 47, 100);
Control ExtruderFwd(42, 100, 100);
Control ExtruderRwd(43, 100, 100);
Control Conveyor(41, 35, 100);
Control Grater(46, 100, 100);
Control Cutter(34, 100, 100);
Control TimerCutterDelay(101, 101, 101);
Control CuttingDelay(100, 100, 100);
Control CheeseDelay(100, 100, 100);

int grindTimeAdd = 10;
int mixingTimeAdd = 20;
int sliceTimeAdd = 30;
int sliceIntervalTimeAdd = 40;

void saveSettings()
{
    EEPROM.updateDouble(grindTimeAdd, parametersTimer[0]);         // Grinder
    EEPROM.updateDouble(mixingTimeAdd, parametersTimer[1]);        // Cooking
    EEPROM.updateDouble(sliceTimeAdd, parametersTimer[2]);         // Door Open
    EEPROM.updateDouble(sliceIntervalTimeAdd, parametersTimer[3]); // Door Close
}

void loadSettings()
{
    parametersTimer[0] = EEPROM.readDouble(grindTimeAdd);
    parametersTimer[1] = EEPROM.readDouble(mixingTimeAdd);
    parametersTimer[2] = EEPROM.readDouble(sliceTimeAdd);
    parametersTimer[3] = EEPROM.readDouble(sliceIntervalTimeAdd);
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

void setTimer()
{
    Grinder.setTimer(secondsToHHMMSS(parametersTimer[0] * 60));
    ExtruderFwd.setTimer(secondsToHHMMSS(parametersTimer[1] * 60));
    ExtruderRwd.setTimer("000010");
    Conveyor.setTimer("000010");
    Grater.setTimer("000010");
    Cutter.setTimer(secondsToHHMMSS(parametersTimer[2]));
    TimerCutterDelay.setTimer(secondsToHHMMSS(parametersTimer[3]));
    CuttingDelay.setTimer(secondsToHHMMSS(60));
    CheeseDelay.setTimer(secondsToHHMMSS(60));
}

bool testRunCut = false;
void runAllTest()
{
    // Grinder.run();
    // ExtruderFwd.run();
    // ExtruderRwd.run();
    // Conveyor.run();
    // Grater.run();
    if (testRunCut == true)
    {
        RunCut();
    }
}
int RunAutoCommand = 0;
void runAutoFunc()
{
    switch (RunAutoCommand)
    {
    case 1:
        RunGrinder();
        break;
    case 2:
        RunMixer();
        break;
    case 3:
        CutterStartDelay();
        
        break;
    case 4:
        CheeseStartDelay();
        break;
    case 5:
        
        RunExtrude();
        break;

    default:
        runAutoFlag = false;
        stopAll();
        break;
    }
}

void stopAll()
{

    RunAutoCommand = 0;
    runAutoFlag = false;
    Grinder.stop();
    Grinder.relayOff();
    ExtruderFwd.stop();
    ExtruderFwd.relayOff();
    ExtruderRwd.stop();
    ExtruderRwd.relayOff();
    Conveyor.stop();
    Conveyor.relayOff();
    Grater.stop();
    Grater.relayOff();
    Cutter.stop();
    Cutter.relayOff();
    TimerCutterDelay.stop();
    TimerCutterDelay.relayOff();

    CuttingDelay.stop();
    CheeseDelay.stop();
}

void RunGrinder()
{
    Grinder.run();
    ExtruderFwd.relayOn();
    if (Grinder.isTimerCompleted() == true)
    {
        RunAutoCommand = 2;
        ExtruderFwd.start();
    }
}

int cutStat = 0;
void RunMixer()
{
    ExtruderFwd.run();
    if (ExtruderFwd.isTimerCompleted() == true)
    {
        delay(300);
        ExtruderRwd.relayOn();
        Conveyor.relayOn();
        
CuttingDelay.start();  RunAutoCommand = 3;
    }
}

void CutterStartDelay(){
    CuttingDelay.run();
    if (CuttingDelay.isTimerCompleted() == true)
    {
        TimerCutterDelay.start();
        cutStat = 0;
        CheeseDelay.start();
        RunAutoCommand = 4;
    }
}
void CheeseStartDelay(){
    CheeseDelay.run();
    RunExtrude();
    if (CheeseDelay.isTimerCompleted() == true)
    {
        Grater.relayOn();
        RunAutoCommand = 5;
    }
}

void RunNormalExtruding(){
    RunExtrude();
}

void RunExtrude()
{
    RunCut();
}

void RunCut()
{
    switch (cutStat)
    {
    case 0:
        TimerCutterDelay.run();
        if (TimerCutterDelay.isTimerCompleted() == true)
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
        // save the last time you blinked the LED
        previousMillis2 = currentMillis2;
        Cutter.relayOff();
        TimerCutterDelay.start();
        cutStat = 0;
    }
    else
    {
        Cutter.relayOn();
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
                setTimer();
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
            stopAll();
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
                if (Conveyor.getMotorState() == true)
                {
                    Conveyor.relayOff();
                }
                if (Grinder.getMotorState() == false)
                {
                    Grinder.relayOn();
                }
                else
                {
                    Grinder.relayOff();
                }
            }
            else if (currentTestMenuScreen == 1)
            {
                if (ExtruderFwd.getMotorState() == false)
                {
                    ExtruderFwd.relayOn();
                }
                else
                {
                    ExtruderFwd.relayOff();
                }
            }
            else if (currentTestMenuScreen == 2)
            {
                if (ExtruderRwd.getMotorState() == false)
                {
                    ExtruderRwd.relayOn();
                }
                else
                {
                    ExtruderRwd.relayOff();
                }
            }
            else if (currentTestMenuScreen == 3)
            {
                if (testRunCut == true)
                {
                    testRunCut = false;
                }
                else
                {
                    testRunCut = true;
                }
            }
            else if (currentTestMenuScreen == 4)
            {
                if (Grater.getMotorState() == false)
                {
                    Grater.relayOn();
                }
                else
                {
                    Grater.relayOff();
                }
            }
            else if (currentTestMenuScreen == 5)
            {
                if (Grinder.getMotorState() == true)
                {
                    Grinder.relayOff();
                }

                if (Conveyor.getMotorState() == false)
                {
                    Conveyor.relayOn();
                }
                else
                {
                    Conveyor.relayOff();
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
                Grinder.start();
                RunAutoCommand = 1;
                refreshScreen = 1;
            }
            else if (currentScreen == 2)
            {
                testMenuFlag = true;
            }
        }
    }
}

void PrintRunAuto(String job, char *time)
{
    lcd.clear();
    lcd.print("Running Auto");
    lcd.setCursor(0, 1);
    lcd.print("Status: " + job);
    lcd.setCursor(0, 2);
    lcd.print("Timer: ");
    lcd.setCursor(7, 2);
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
        if (runAutoFlag == true && RunAutoCommand == 1)
        {
            PrintRunAuto("Grinding", Grinder.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 2)
        {
            PrintRunAuto("Mixing", ExtruderFwd.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 3)
        {
            PrintRunAuto("Cutting Delay", CuttingDelay.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 4)
        {
            PrintRunAuto("Cheese Delay", CheeseDelay.getTimeRemaining());
        }
        else if (runAutoFlag == true && RunAutoCommand == 5)
        {
            PrintRunAuto("Extruding", "N/A");
        }
    }
    else if (testMenuFlag == true)
    {
        lcd.clear();
        lcd.print(TestMenuScreen[currentTestMenuScreen]);

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
    // Encoder Setup
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

    // saveSettings(); // Disable upon Initialiaze
    loadSettings();
    setTimer();
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

    if (runAutoFlag == true)
    {
        runAutoFunc();
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            // save the last time you blinked the LED
            previousMillis = currentMillis;
            refreshScreen = true;
        }
    }

    if (testMenuFlag == true)
    {
        runAllTest();
    }
}