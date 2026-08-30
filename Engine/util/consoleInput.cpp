#include "consoleInput.hpp"

#include <iostream>
#include <limits>

#if defined(_WIN32)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace consoleInput {

std::string readPassword()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string password;

#if defined(_WIN32)

    char ch = 0;
    while ((ch = static_cast<char>(_getch())) != '\r' && ch != '\n') {
        if (ch == '\b') { // backspace
            if (!password.empty()) {
                password.pop_back();
            }
            continue;
        }
        password.push_back(ch);
    }
    std::cout << std::endl;
#else
    if (!isatty(fileno(stdin))) {
        // Input is piped/redirected (e.g. automated tests) rather than an
        // interactive terminal. There's no echo to suppress, so just read
        // normally.
        std::getline(std::cin, password);
        return password;
    }

    termios originalSettings{};
    termios noEchoSettings{};

    tcgetattr(STDIN_FILENO, &originalSettings);
    noEchoSettings = originalSettings;
    noEchoSettings.c_lflag &= ~static_cast<tcflag_t>(ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &noEchoSettings);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings);

    std::cout << std::endl; // the Enter keypress wasn't echoed while masked
#endif

    return password;
}

} // namespace consoleInput
