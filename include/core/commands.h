#ifndef COMMANDS_H
#define COMMANDS_H



#include "help.h"
#include "display.h"
#include "filesystem.h"
#include "network.h"
#include "theme.h"
#include "config.h"
#include "pug.h"
#include "ball.h"
#include "timeutils.h"
#include "kernel.h"
#include "grapher.h"
#include "mirror.h"
#include "pingpong.h"
#include "d20.h"
#include "userspace/calc.h"
#include "userspace/fetch.h"
#include "userspace/ping.h"
#include "userspace/utils.h"
#include "userspace/sysinfo.h"
#include <esp_system.h>
#include <WiFi.h>
#include <math.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <Arduino.h>

void runCommand(const std::string& cmd);
void showVersion();
void showHelp();
void showHelpOS();
void addToHistory(const std::string& cmd);
void showHistory();

void processCommand(const std::string& args);
void showSystemStats();
void killProcessCmd(int pid);

extern bool promptPrinted;

#endif