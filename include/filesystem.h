#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include <string>

bool initFilesystem();
void writeFile(const std::string& name, const std::string& data);
void appendFile(const std::string& name, const std::string& data);
void readFile(const std::string& name);
void deleteFile(const std::string& name);
void listFiles();
bool renameFile(const std::string& oldName, const std::string& newName);
bool copyFile(const std::string& src, const std::string& dst);

#endif