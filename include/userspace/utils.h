#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string base64Encode(const std::string& input);
std::string base64Decode(const std::string& input);
void base64Command(const std::string& operation, const std::string& text);



void hexCommand(const std::string& numStr);
void binCommand(const std::string& numStr);

void echoCommand(const std::string& text);

#endif