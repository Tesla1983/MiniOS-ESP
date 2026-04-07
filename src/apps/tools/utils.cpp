#include "utils.h"
#include "display.h"
#include <math.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>



const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64Encode(const std::string& input) {
    std::string output = "";
    int val = 0;
    int valb = -6;
    
    for (unsigned int i = 0; i < input.length(); i++) {
        unsigned char c = input[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output += base64_chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        output += base64_chars[((val << 8) >> (valb + 8)) & 0x3F];
    }
    
    while (output.length() % 4) {
        output += '=';
    }
    
    return output;
}

std::string base64Decode(const std::string& input) {
    std::string output = "";
    int val = 0;
    int valb = -8;
    
    for (unsigned int i = 0; i < input.length(); i++) {
        char c = input[i];
        if (c == '=') break;
        
        int index = -1;
        for (int j = 0; j < 64; j++) {
            if (base64_chars[j] == c) {
                index = j;
                break;
            }
        }
        
        if (index == -1) continue;
        
        val = (val << 6) + index;
        valb += 6;
        
        if (valb >= 0) {
            output += char((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return output;
}

void base64Command(const std::string& operation, const std::string& text) {
    if (operation == "encode") {
        std::string encoded = base64Encode(text);
        printLine("Encoded: " + encoded);
    } else if (operation == "decode") {
        std::string decoded = base64Decode(text);
        printLine("Decoded: " + decoded);
    } else {
        printLine("Usage: base64 encode <text>");
        printLine("       base64 decode <text>");
    }
}

void hexCommand(const std::string& numStr) {
    long num = std::stol(numStr);
    char hexStr[20];
    sprintf(hexStr, "0x%lX", num);
    printLine("Decimal: " + std::to_string(num));
    printLine("Hexadecimal: " + std::string(hexStr));
}

void binCommand(const std::string& numStr) {
    long num = std::stol(numStr);
    std::string binStr = "";
    
    if (num == 0) {
        binStr = "0";
    } else {
        long temp = num;
        while (temp > 0) {
            binStr = std::to_string(temp % 2) + binStr;
            temp /= 2;
        }
    }
    
    printLine("Decimal: " + std::to_string(num));
    printLine("Binary: 0b" + binStr);
}

void echoCommand(const std::string& text) {
    printLine(text);
}