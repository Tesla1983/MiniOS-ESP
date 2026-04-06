#ifndef GRAPHER_H
#define GRAPHER_H

#include <Arduino.h>
#include <string>

bool evaluateWithX(const std::string& expression, float xValue, float& result);
void funcToGraph(const std::string& expression, const std::string& lineColour);

#endif