#include "commands.h"
#include "display.h"
#include "filesystem.h"
#include "network.h"
#include "theme.h"
#include "config.h"
#include "pug.h"
#include "timeutils.h"
#include "kernel.h"
#include "grapher.h"
#include <esp_system.h>
#include <WiFi.h>
#include <math.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

#define PI 3.14159265358979323846
#define E  2.71828182845904523536
#define HISTORY_SIZE 10  
const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::string commandHistory[HISTORY_SIZE];
int historyIndex = 0;
int historyCount = 0;





void showVersion() {
    printLine("MiniOS " + std::string(OS_VERSION));
    printLine("Repository: github.com/VuqarAhadli");
}


void addToHistory(const std::string& cmd) {
    if (cmd.length() == 0) return;
    if (historyCount > 0 && commandHistory[(historyIndex - 1 + HISTORY_SIZE) % HISTORY_SIZE] == cmd) return;
    commandHistory[historyIndex] = cmd;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
}

void showHistory() {
    if (historyCount == 0) {
        printLine("No command history.");
        return;
    }
    printLine("Command history:");
    int start = (historyIndex - historyCount + HISTORY_SIZE) % HISTORY_SIZE;
    for (int i = 0; i < historyCount; i++) {
        int idx = (start + i) % HISTORY_SIZE;
        printLine(std::to_string(i + 1) + ": " + commandHistory[idx]);
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


void showHelp() {
    printLine("MiniOS Command Help");
    printLine("");
    printLine("  help file     - File commands");
    printLine("  help system   - System commands");
    printLine("  help network  - Network commands");
    printLine("  help utils    - Utility commands");
    printLine("  help time     - Time commands");
    printLine("  help display  - Display commands");
    printLine("  help os       - OS management");
}

void showHelpFile() {
    printLine("File Commands:");
    printLine("  write <file> <text>   - Write text");
    printLine("  append <file> <text>  - Append text");
    printLine("  read <file>           - Read file");
    printLine("  delete <file>         - Delete file (alias: rm)");
    printLine("  ls                    - List files (alias: dir)");
    printLine("  mv <old> <new>        - Rename file (alias: rename)");
    printLine("  cp <src> <dst>        - Copy file (alias: copy)");
}

void showHelpSystem() {
    printLine("System Commands:");
    printLine("  mem       - Memory info (alias: free)");
    printLine("  uptime    - System uptime");
    printLine("  reboot    - Restart device (alias: restart)");
    printLine("  fetch     - System info (alias: neofetch)");
    printLine("  os        - OS logo (alias: logo)");
    printLine("  version   - OS version (alias: ver)");
    printLine("  clear     - Clear display (alias: cls)");
    printLine("  history   - Command history (alias: hist)");
}

void showHelpNetwork() {
    printLine("Network Commands:");
    printLine("  wifi              - Connect to WiFi");
    printLine("  disconnect        - Disconnect WiFi");
    printLine("  scanwifi          - Scan networks");
    printLine("  ifconfig          - Network info");
    printLine("  ping <host>       - Ping host");
    printLine("  nslookup <host>   - DNS lookup");
    printLine("  curl <url>        - Fetch URL");
    printLine("  curl -v <url>     - Verbose mode");
}

void showHelpUtils() {
    printLine("Utility Commands:");
    printLine("  calc <expr>                 - Calculator");
    printLine("  hex <number>                - Dec to hex");
    printLine("  bin <number>                - Dec to bin");
    printLine("  base64 encode <text>        - Encode Base64");
    printLine("  base64 decode <text>        - Decode Base64");
    printLine("  graph <expression> <colour> - Graph function");
    printLine("  echo <text>                 - Print text");
}

void showHelpTime() {
    printLine("Time Commands:");
    printLine("  time            - Current time");
    printLine("  synctime        - Sync with NTP");
    printLine("  calendar        - Show calendar");
    printLine("  timer <sec>     - Countdown timer");
    printLine("  stopwatch       - Elapsed timer");
    printLine("  alarm <HH:MM>   - Set alarm");
}

void showHelpOS() {
    printLine("OS Commands:");
    printLine("  ps / processes - List processes");
    printLine("  sysstat / stat - System stats");
    printLine("  kill <pid>     - Kill process");
}

void showHelpDisplay() {
    printLine("Display Commands:");
    printLine("  themes          - List themes");
    printLine("  theme <n>       - Select theme");
    printLine("  screensaver <n> - Run screensaver");
    printLine("  pug             - Show pug image");
}


int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3;
    return 0;
}

bool isRightAssociative(char op) {
    return (op == '^');
}

float applyOp(float a, float b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': 
            if (b == 0) {
                printLine("Error: Division by zero");
                return 0;
            }
            return a / b;
        case '%': 
            if (b == 0) {
                printLine("Error: Modulo by zero");
                return 0;
            }
            return (int)a % (int)b;
        case '^': return pow(a, b);
    }
    return 0;
}

bool isFunction(const std::string& expr, int pos, const std::string& func) {
    int len = func.length();
    if (pos + len > (int)expr.length()) return false;
    std::string substr = expr.substr(pos, len);
    std::transform(substr.begin(), substr.end(), substr.begin(), ::tolower);
    return substr == func;
}

std::string getFunctionName(const std::string& expr, int pos) {
    std::string functions[] = {
        "sqrt", "sin", "cos", "tan", "asin", "acos", "atan",
        "sinh", "cosh", "tanh", "log", "ln", "exp",
        "abs", "ceil", "floor", "round"
    };
    for (int i = 0; i < 17; i++) {
        if (isFunction(expr, pos, functions[i])) {
            return functions[i];
        }
    }
    return "";
}

float applyFunction(const std::string& func, float value) {
    if (func == "sqrt") return sqrt(value);
    if (func == "sin") return sin(value);
    if (func == "cos") return cos(value);
    if (func == "tan") return tan(value);
    if (func == "asin") return asin(value);
    if (func == "acos") return acos(value);
    if (func == "atan") return atan(value);
    if (func == "sinh") return sinh(value);
    if (func == "cosh") return cosh(value);
    if (func == "tanh") return tanh(value);
    if (func == "log") return log10(value);
    if (func == "ln") return log(value);
    if (func == "exp") return exp(value);
    if (func == "abs") return abs(value);
    if (func == "ceil") return ceil(value);
    if (func == "floor") return floor(value);
    if (func == "round") return round(value);
    return value;
}

void calc(const std::string& expression_in) {
    std::string expression = expression_in;
    size_t pos = expression.find("pi");
    while (pos != std::string::npos) {
        expression.replace(pos, 2, std::to_string(PI));
        pos = expression.find("pi", pos + std::to_string(PI).length());
    }
    pos = expression.find("e");
    while (pos != std::string::npos) {
        expression.replace(pos, 1, std::to_string(E));
        pos = expression.find("e", pos + std::to_string(E).length());
    }
    
    if (expression.length() == 0) {
        printLine("Error: Empty expression");
        return;
    }
    
    float values[50];
    char ops[50];
    std::string functions[50];
    int vTop = -1;
    int oTop = -1;
    int fTop = -1;
    
    int n = expression.length();
    
    for (int i = 0; i < n; i++) {
        std::string funcName = getFunctionName(expression, i);
        if (funcName.length() > 0) {
            i += funcName.length();
            functions[++fTop] = funcName;
            continue;
        }
        
        if (expression[i] == '(') {
            ops[++oTop] = '(';
        }
        else if (expression[i] == ')') {
            while (oTop >= 0 && ops[oTop] != '(') {
                float b = values[vTop--];
                float a = values[vTop--];
                char op = ops[oTop--];
                values[++vTop] = applyOp(a, b, op);
            }
            if (oTop >= 0) oTop--;
            
            if (fTop >= 0) {
                float val = values[vTop--];
                val = applyFunction(functions[fTop--], val);
                values[++vTop] = val;
            }
        }
        else if (isdigit(expression[i]) || expression[i] == '.') {
            float num = 0;
            float decimal = 0;
            bool hasDecimal = false;
            int decimalPlaces = 0;
            
            while (i < n && (isdigit(expression[i]) || expression[i] == '.')) {
                if (expression[i] == '.') {
                    hasDecimal = true;
                } else {
                    if (hasDecimal) {
                        decimal = decimal * 10 + (expression[i] - '0');
                        decimalPlaces++;
                    } else {
                        num = num * 10 + (expression[i] - '0');
                    }
                }
                i++;
            }
            i--;
            
            if (hasDecimal && decimalPlaces > 0) {
                num += decimal / pow(10, decimalPlaces);
            }
            
            values[++vTop] = num;
        }
        else if (expression[i] == '-' && 
                (i == 0 || expression[i-1] == '(' || expression[i-1] == '+' || 
                 expression[i-1] == '-' || expression[i-1] == '*' || 
                 expression[i-1] == '/' || expression[i-1] == '^')) {
            values[++vTop] = 0;
            ops[++oTop] = '-';
        }
        else if (expression[i] == '+' || expression[i] == '-' || 
                 expression[i] == '*' || expression[i] == '/' || 
                 expression[i] == '%' || expression[i] == '^') {
            while (oTop >= 0 && ops[oTop] != '(' && 
                   (precedence(ops[oTop]) > precedence(expression[i]) ||
                    (precedence(ops[oTop]) == precedence(expression[i]) && 
                     !isRightAssociative(expression[i])))) {
                float b = values[vTop--];
                float a = values[vTop--];
                char op = ops[oTop--];
                values[++vTop] = applyOp(a, b, op);
            }
            ops[++oTop] = expression[i];
        }
        else {
            printLine("Error: Invalid character '" + std::string(1, expression[i]) + "'");
            return;
        }
    }
    
    while (oTop >= 0) {
        if (ops[oTop] == '(') {
            printLine("Error: Mismatched parentheses");
            return;
        }
        float b = values[vTop--];
        float a = values[vTop--];
        char op = ops[oTop--];
        values[++vTop] = applyOp(a, b, op);
    }
    
    if (vTop != 0) {
        printLine("Error: Invalid expression");
        return;
    }
    
    float result = values[vTop];
    
    if (result == (int)result && abs(result) < 1000000) {
        printLine("Result: " + std::to_string((int)result));
    } else {
        char buf[32];
        sprintf(buf, "Result: %.6f", result);
        printLine(buf);
    }
}




void showMem() {
    uint32_t fbytes = ESP.getFreeHeap();
    uint32_t mibytes = ESP.getMinFreeHeap();
    uint32_t mabytes = ESP.getMaxAllocHeap();
    float fkb = fbytes / 1024.0;
    float mikb = mibytes / 1024.0;
    float makb = mabytes / 1024.0;

    char buf[100];
    sprintf(buf, "Free Heap: %u bytes (%.2f KB)", ESP.getFreeHeap(), fkb);
    printLine(buf);
    sprintf(buf, "Min Free Heap: %u bytes (%.2f KB)", ESP.getMinFreeHeap(), mikb);
    printLine(buf);
    sprintf(buf, "Max Alloc Heap: %u bytes (%.2f KB)", ESP.getMaxAllocHeap(), makb);
    printLine(buf);
}


void showUptime() {
    unsigned long s = millis() / 1000;
    unsigned long h = s / 3600;
    unsigned long m = (s % 3600) / 60;
    unsigned long sec = s % 60;
    printLine("Uptime: " + std::to_string(h) + "h " + std::to_string(m) + "m " + std::to_string(sec) + "s");
}

void doReboot() {
    printLine("Rebooting...");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP.restart();
}

void showChipInfo() {
    printLine("Chip Model: " + std::string(ESP.getChipModel()));
    printLine("Chip Cores: " + std::to_string(ESP.getChipCores()));
    printLine("Chip Revision: " + std::to_string(ESP.getChipRevision()));
}
void showCPUInfo() {
    printLine(
        "CPU: " +
        std::to_string(ESP.getCpuFreqMHz()) + " MHz"
    );
}
void showFlashInfo() {
    uint32_t flashSize = ESP.getFlashChipSize() / 1024 / 1024;
    printLine(
        "Flash: " +
        std::to_string(flashSize) + " MB"
    );
    printLine("Flash Speed: " + std::to_string(ESP.getFlashChipSpeed() / 1000000) + " MHz");
}
void showWiFiInfo() {
    if (WiFi.isConnected()) {
        printLine("WiFi RSSI: " + std::to_string(WiFi.RSSI()) + " dBm");
        printLine("WiFi Channel: " + std::to_string(WiFi.channel()));
        printLine("MAC: " + std::string(WiFi.macAddress().c_str()));
    } else {
        printLine("WiFi: Disconnected");
    }
}

void fetch() {
    showLogo();
    printLine("");
    showUptime();
    showMem();
    showChipInfo();
    showFlashInfo();
    showCPUInfo();
    showWiFiInfo();
    
    printLine("");
    
    int startX = 5;
    int startY = tft.getCursorY() + 5; 
    int blockWidth = 15;
    int blockHeight = 10;

    uint16_t colors1[8] = {0x0000,0x7800,0x03E0,0x7BE0,0x0010,0x780F,0x03EF,0xC618};
    for (int i=0;i<8;i++) tft.fillRect(startX + i*blockWidth, startY, blockWidth, blockHeight, colors1[i]);

    
    startY += blockHeight;
    uint16_t colors2[8] = {0x4208,0xF800,0x07E0,0xFFE0,0x001F,0xF81F,0x07FF,0xFFFF};
    for (int i=0;i<8;i++) tft.fillRect(startX + i*blockWidth, startY, blockWidth, blockHeight, colors2[i]);

    
    startY += blockHeight;
    tft.setCursor(startX, startY + 5);
    currentCursorY = startY; 


    }

void echoCommand(const std::string& text) {
    printLine(text);
}


struct CommandArgs {
    std::string cmd;
    std::string arg1;
    std::string arg2;
    std::string rest;
};

CommandArgs parseCommand(const std::string& input_in) {
    CommandArgs args;
    std::string input = input_in;
    
    input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
    input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
    
    size_t firstSpace = input.find(' ');
    if (firstSpace == std::string::npos) {
        args.cmd = input;
        return args;
    }
    
    args.cmd = input.substr(0, firstSpace);
    std::string remainder = input.substr(firstSpace + 1);
    
    remainder.erase(0, remainder.find_first_not_of(" \t\n\r\f\v"));
    remainder.erase(remainder.find_last_not_of(" \t\n\r\f\v") + 1);
    
    size_t secondSpace = remainder.find(' ');
    if (secondSpace == std::string::npos) {
        args.arg1 = remainder;
        return args;
    }
    
    args.arg1 = remainder.substr(0, secondSpace);
    args.rest = remainder.substr(secondSpace + 1);
    args.rest.erase(0, args.rest.find_first_not_of(" \t\n\r\f\v"));
    args.rest.erase(args.rest.find_last_not_of(" \t\n\r\f\v") + 1);
    
    size_t thirdSpace = args.rest.find(' ');
    if (thirdSpace != std::string::npos) {
        args.arg2 = args.rest.substr(0, thirdSpace);
        args.rest = args.rest.substr(thirdSpace + 1);
        args.rest.erase(0, args.rest.find_first_not_of(" \t\n\r\f\v"));
        args.rest.erase(args.rest.find_last_not_of(" \t\n\r\f\v") + 1);
    } else if (args.rest.length() > 0) {
        args.arg2 = args.rest;
        args.rest = "";
    }
    
    return args;
}

void runCommand(const std::string& cmd_in) {
    std::string cmd = cmd_in;
    cmd.erase(0, cmd.find_first_not_of(" \t\n\r\f\v"));
    cmd.erase(cmd.find_last_not_of(" \t\n\r\f\v") + 1);
    
    if (cmd.length() == 0){
        if(currentCursorY>=MAX_Y){
            clearScreen();
            currentCursorY=0;
        }
        return;
    }
    
    addToHistory(cmd);
    
    CommandArgs args = parseCommand(cmd);
    std::string baseCmd = args.cmd;
    std::transform(baseCmd.begin(), baseCmd.end(), baseCmd.begin(), ::tolower);
    
    if (baseCmd == "write") {
        if (args.arg1.length() == 0 || args.rest.length() == 0) {
            printLine("Usage: write <filename> <text>");
            return;
        }
        writeFile(args.arg1, args.arg2 + args.rest);
    }
    else if (baseCmd == "append") {
        if (args.arg1.length() == 0 || args.rest.length() == 0) {
            printLine("Usage: append <filename> <text>");
            return;
        }
        appendFile(args.arg1, args.arg2 + args.rest);
    }
    else if (baseCmd == "read") {
        if (args.arg1.length() == 0) {
            printLine("Usage: read <filename>");
            return;
        }
        readFile(args.arg1);
    }
    else if (baseCmd == "delete" || baseCmd == "rm") {
        if (args.arg1.length() == 0) {
            printLine("Usage: delete <filename>");
            return;
        }
        deleteFile(args.arg1);
    }
    else if (baseCmd == "ls" || baseCmd == "dir") {
        listFiles();
    }
    else if (baseCmd == "mv" || baseCmd == "rename") {
        if (args.arg1.length() == 0 || args.arg2.length() == 0) {
            printLine("Usage: mv <old> <new>");
            return;
        }
        renameFile(args.arg1, args.arg2);
    }
    else if (baseCmd == "cp" || baseCmd == "copy") {
        if (args.arg1.length() == 0 || args.arg2.length() == 0) {
            printLine("Usage: cp <src> <dst>");
            return;
        }
        copyFile(args.arg1, args.arg2);
    }
    else if (baseCmd == "wifi") {
        if (args.arg1 == "disconnect") {
            disconnectWiFi();
        } else {
            connectWiFi();
        }
    }
    else if (baseCmd == "disconnect") {
        disconnectWiFi();
    }
    else if (baseCmd == "scanwifi" || baseCmd == "wifiscan") {
        scanWiFi();
    }
    else if (baseCmd == "ifconfig" || baseCmd == "netinfo" || baseCmd == "ipconfig") {
        showNetworkInfo();
    }
    else if (baseCmd == "curl") {
        if (args.arg1.length() == 0) {
            printLine("Usage: curl [-v] <url>");
            return;
        }
        
        if (args.arg1 == "-v") {
            if (args.rest.length() == 0) {
                printLine("Usage: curl -v <url>");
                return;
            }
            curlURLVerbose(args.rest);
        } else {
            curlURL(args.arg1 + (args.rest.length() > 0 ? " " + args.rest : ""));
        }
    }
    else if (baseCmd == "ping") {
        if (args.arg1.length() == 0) {
            printLine("Usage: ping <host>");
            return;
        }
        pingHost(args.arg1);
    }
    else if (baseCmd == "nslookup" || baseCmd == "dns") {
        if (args.arg1.length() == 0) {
            printLine("Usage: nslookup <host>");
            return;
        }
        dnsLookup(args.arg1);
    }
    else if (baseCmd == "mem" || baseCmd == "memory") {
        showMem();
    }
    else if (baseCmd == "uptime") {
        showUptime();
    }
    else if (baseCmd == "reboot" || baseCmd == "restart") {
        doReboot();
    }
    else if (baseCmd == "fetch" || baseCmd == "neofetch" || baseCmd == "fastfetch" ) {
        fetch();
    }
    else if (baseCmd == "os" || baseCmd == "logo") {
        showLogo();
    }
    else if (baseCmd == "version" || baseCmd == "ver") {
        showVersion();
    }
    else if (baseCmd == "clear" || baseCmd == "cls") {
        clearScreen();
        
    }
    else if (baseCmd == "history" || baseCmd == "hist") {
        showHistory();
    }
    else if (baseCmd == "ps" || baseCmd == "processes" || baseCmd == "top") {
        listProcesses();
    }
    else if (baseCmd == "sysstat" || baseCmd == "stat") {
        showSystemStats();
    }
    else if (baseCmd == "kill") {
        if (args.arg1.length() == 0) {
            printLine("Usage: kill <pid>");
            return;
        }
        int pid = 0;
        try {
            pid = std::stoi(args.arg1);
        } catch (...) {
            pid = 0;
        }
        if (pid <= 0) {
            printLine("Invalid PID");
            return;
        }
        killProcess(pid);
    }
    else if (baseCmd == "echo") {
        echoCommand(args.arg1 + (args.rest.length() > 0 ? " " + args.rest : ""));
    }
    else if (baseCmd == "calc") {
        if (args.arg1.length() == 0) {
            printLine("Usage: calc <expression>");
            return;
        }
        calc(args.arg1 + (args.rest.length() > 0 ? " " + args.rest : ""));
    }
    else if (baseCmd == "hex") {
        if (args.arg1.length() == 0) {
            printLine("Usage: hex <number>");
            return;
        }
        hexCommand(args.arg1);
    }
    else if (baseCmd == "bin") {
        if (args.arg1.length() == 0) {
            printLine("Usage: bin <number>");
            return;
        }
        binCommand(args.arg1);
    }
    else if (baseCmd == "base64") {
        if (args.arg1.length() == 0) {
            printLine("Usage: base64 encode <text>");
            printLine("       base64 decode <text>");
            return;
        }
        
        std::string operation = args.arg1;
        std::string text = args.arg2;
        if (args.rest.length() > 0) {
            text = args.arg2 + " " + args.rest;
        }
        
        if (text.length() == 0) {
            printLine("Usage: base64 encode <text>");
            printLine("       base64 decode <text>");
            return;
        }
        
        base64Command(operation, text);
    }   
    else if (baseCmd == "time" || baseCmd == "date") {
        printLine(getTime());
    }
    else if (baseCmd == "synctime" || baseCmd == "ntpupdate") {
        syncTime();
    }
    else if (baseCmd == "calendar" || baseCmd == "cal") {
        showCalendar();
    }
    else if (baseCmd == "timer") {
        if (args.arg1.length() == 0) {
            printLine("Usage: timer <seconds>");
            return;
        }
        int seconds = 0;
        try {
            seconds = std::stoi(args.arg1);
        } catch (...) {
            seconds = 0;
        }
        if (seconds <= 0) {
            printLine("Invalid time");
            return;
        }
        timerCommand(seconds);
    }
    else if (baseCmd == "stopwatch" || baseCmd == "sw") {
        stopwatchCommand();
    }
    else if (baseCmd == "alarm") {
        if (args.arg1.length() == 0) {
            if (systemAlarm.active) {
                printLine("Alarm set for " + std::to_string(systemAlarm.hour) + ":" + 
                         (systemAlarm.minute < 10 ? "0" : "") + std::to_string(systemAlarm.minute));
            } else {
                printLine("No alarm set.");
            }
        } else {
            setAlarm(args.arg1);
        }
    }
    else if (baseCmd == "themes") {
        listThemes();
    }
    else if (baseCmd == "theme") {
        if (args.arg1.length() == 0) {
            printLine("Usage: theme <number>");
            listThemes();
            return;
        }
        setTheme(args.arg1);
    }
    else if (baseCmd == "pug") {
        displayPug();
    }
    else if (baseCmd == "screensaver" || baseCmd == "ss") {
        if (args.arg1.length() == 0) {
            printLine("Usage: screensaver <mode>");
            printLine("Available modes: 1-7");
            return;
        }
        
        int mode = std::stoi(args.arg1);
        if (mode < 1 || mode > 7) {
            printLine("Invalid mode. Use 1-7.");
            return;
        }
        
        screensaver(mode);
    }
    else if (baseCmd == "graph" || baseCmd == "plot") {
        if (args.arg1.length() == 0) {
            printLine("Usage: graph <expression> [color]");
            printLine("Example: graph sin(x) red");
            return;
        }
        std::string color = args.arg2.length() > 0 ? args.arg2 : "blue";
        funcToGraph(args.arg1, color);
    }
    else if (baseCmd == "username" || baseCmd == "name") {
        if (args.arg1.length() == 0 || args.arg2.length() != 0) {
            printLine("Usage: username <name>");
            return;
        }
        setDeviceName(args.arg1);
    }
    else if (baseCmd == "help" || baseCmd == "h") {
        if (args.arg1.length() == 0) {
            showHelp();
        } else if (args.arg1 == "file") {
            showHelpFile();
        } else if (args.arg1 == "system") {
            showHelpSystem();
        } else if (args.arg1 == "network") {
            showHelpNetwork();
        } else if (args.arg1 == "utils") {
            showHelpUtils();
        } else if (args.arg1 == "time") {
            showHelpTime();
        } else if (args.arg1 == "display") {
            showHelpDisplay();
        } else if (args.arg1 == "os") {
            showHelpOS();
        } else {
            printLine("Unknown help topic: " + args.arg1);
            showHelp();
        }
    }
    else {
        printLine("Unknown command: " + baseCmd);
        printLine("Type 'help' for available commands");
    }
}




void processCommand(String args) {
    listProcesses();
}

void showSystemStats() {
    printSystemStats();
}

void killProcessCmd(int pid) {
    killProcess(pid);
}
