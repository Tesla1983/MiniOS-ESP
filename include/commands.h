#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>
#include <string>

void runCommand(const std::string& cmd);
void showVersion();
void showHelp();
void showHelpOS();
int precedence(char op);
bool isRightAssociative(char op);
float applyOp(float a, float b, char op);
bool isFunction(const std::string& expr, int pos, const std::string& func);
std::string getFunctionName(const std::string& expr, int pos);
float applyFunction(const std::string& func, float value);
void calc(const std::string& expression);
void showMem();
void showUptime();
void doReboot();
void showChipInfo();
void showCPUInfo();
void showFlashInfo();
void showWiFiInfo();
void fetch();
void echoCommand(const std::string& text);
void addToHistory(const std::string& cmd);
void showHistory();

void hexCommand(const std::string& numStr);
void binCommand(const std::string& numStr);
void base64Command(const std::string& operation, const std::string& text);
std::string base64Encode(const std::string& input);
std::string base64Decode(const std::string& input);


void processCommand(const std::string& args);
void showSystemStats();
void killProcessCmd(int pid);

extern bool promptPrinted;

#endif