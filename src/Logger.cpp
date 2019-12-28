#include "Logger.h"

#include <iostream>
#include <vector>
#include <stdio.h>

#include <unistd.h>

#include "Options.h"

static std::string info_line;

/**
 * @brief Checks if we can display ANSI escape codes through stdout.
 * @details Specifically, checks if the STDOUT file is a TTY.
 *
 * @return True if we can display ANSI through stdout, false otherwise.
 */
static bool isOutAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stdout));

    return ansi_enabled;
}

/**
 * @brief Checks if we can display ANSI escape codes through stderr.
 * @details Specifically, checks if the STDERR file is a TTY.
 *
 * @return True if we can display ANSI through stderr, false otherwise.
 */
static bool isErrAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stderr));

    return ansi_enabled;
}

/**
 * @brief Outputs the current information line.
 * @details Will do nothing if program output is set to QUIET
 */
static void writeInfoLine() {
    if(MapNormalizer::prog_opts.quiet) return;

    if(!info_line.empty()) {
        std::cout << (isOutAnsiEnabled() ? "\33[32m" : "") << "==> " << info_line
                  << (isOutAnsiEnabled() ? "\33[0m" : "");

        // Only print a newline if we don't have ANSI escape codes.
        if(!isOutAnsiEnabled()) std::cout << std::endl;
        std::cout.flush();
    }
}

/**
 * @brief Deletes the last information line from the console, so that we can display the next one.
 * @details Will do nothing if program output is set to QUIET or if ANSI escape
 *          codes are not enabled for STDOUT.
 */
void MapNormalizer::deleteInfoLine() {
    if(prog_opts.quiet) return;

    if(!info_line.empty() && isOutAnsiEnabled()) {
        std::cout << "\33[1000D"; // Go to start of line
        std::cout << "\33[0K";    // Clear the line
        std::cout << "\33[1000D"; // Go to start of line
        std::cout.flush();
    }
}

/**
 * @brief Sets and displays the next information line.
 *
 * @param line The next textual string to use for the information line.
 */
void MapNormalizer::setInfoLine(const std::string& line) {
    info_line = line;

    deleteInfoLine();
    writeInfoLine();
}

/**
 * @brief Writes a warning.
 * @details Outputs to stderr. If requested, prefix is "[WRN] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeWarning(const std::string& message, bool write_prefix)
{
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[33m" : "")
              << (write_prefix ? "[WRN] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cerr.flush();
    writeInfoLine();
}

/**
 * @brief Writes an error.
 * @details Outputs to stderr. If requested, prefix is "[ERR] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeError(const std::string& message, bool write_prefix) {
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[31m" : "")
              << (write_prefix ? "[ERR] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cerr.flush();
    writeInfoLine();
}

/**
 * @brief Writes a output message.
 * @details Outputs to stdout. If requested, prefix is "[OUT] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeStdout(const std::string& message, bool write_prefix) {
    if(prog_opts.quiet) return;

    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[37m" : "")
              << (write_prefix ? "[OUT] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cout.flush();
    writeInfoLine();
}

/**
 * @brief Writes a debug message.
 * @details Outputs to stdout. If requested, prefix is "[DBG] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeDebug(const std::string& message, bool write_prefix) {
    // Do nothing if we don't have verbose output.
    if(!prog_opts.verbose) return;

    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[34m" : "")
              << (write_prefix ? "[DBG] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cout.flush();
    writeInfoLine();
}

