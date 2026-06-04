// Credits: TwilitRealm

#include "port/cli.h"

#include <game/object.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdio>
#include <string>
#include <string_view>

namespace {

struct CliState {
    bool minigameLaunchEnabled = false;
    int minigameNumber = -1;
    int minigameOverlay = DLL_NONE;
    int cpuDifficulty = 1;
};

CliState State;

int minigame_number_to_overlay(int number)
{
    if (number >= 401 && number <= 451) {
        return DLL_m401dll + (number - 401);
    }
    if (number == 453) {
        return DLL_m453dll;
    }
    if (number >= 455 && number <= 463) {
        return DLL_m455dll + (number - 455);
    }
    return DLL_NONE;
}

bool parse_int(std::string_view value, int& out)
{
    int parsed = 0;
    const char* begin = value.data();
    const char* end = value.data() + value.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        return false;
    }

    out = parsed;
    return true;
}

std::string lowercase(std::string_view value)
{
    std::string result(value);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

bool parse_minigame(std::string_view value, int& number, int& overlay)
{
    if (!value.empty() && (value.front() == 'm' || value.front() == 'M')) {
        value.remove_prefix(1);
    }

    int parsedNumber = -1;
    if (!parse_int(value, parsedNumber)) {
        return false;
    }

    overlay = minigame_number_to_overlay(parsedNumber);
    if (overlay == DLL_NONE) {
        return false;
    }

    number = parsedNumber;
    return true;
}

bool parse_cpu_difficulty(std::string_view value, int& difficulty)
{
    int parsed = -1;
    if (parse_int(value, parsed)) {
        if (parsed >= 0 && parsed <= 3) {
            difficulty = parsed;
            return true;
        }
        return false;
    }

    const std::string normalized = lowercase(value);
    if (normalized == "easy") {
        difficulty = 0;
        return true;
    }
    if (normalized == "medium") {
        difficulty = 1;
        return true;
    }
    if (normalized == "hard") {
        difficulty = 2;
        return true;
    }
    if (normalized == "expert") {
        difficulty = 3;
        return true;
    }

    return false;
}

bool take_next_value(int argc, char* argv[], int& index, std::string_view option, std::string_view& outValue)
{
    if (index + 1 >= argc) {
        std::fprintf(stderr, "Missing value for %.*s\n", static_cast<int>(option.size()), option.data());
        return false;
    }

    ++index;
    outValue = argv[index];
    return true;
}

} // namespace

namespace partyboard {

bool parseCommandLine(int argc, char* argv[])
{
    State = {};

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        std::string_view value;

        if (arg == "--minigame" || arg == "-m") {
            if (!take_next_value(argc, argv, i, arg, value)) {
                return false;
            }
        } else if (arg.starts_with("--minigame=")) {
            value = arg.substr(sizeof("--minigame=") - 1);
        } else if (arg.starts_with("-m") && arg.size() > 2) {
            value = arg.substr(2);
        } else if (arg == "--cpu-difficulty") {
            if (!take_next_value(argc, argv, i, arg, value)) {
                return false;
            }
            if (!parse_cpu_difficulty(value, State.cpuDifficulty)) {
                std::fprintf(stderr, "Invalid CPU difficulty '%.*s'. Use easy, medium, hard, expert, or 0-3.\n",
                    static_cast<int>(value.size()), value.data());
                return false;
            }
            continue;
        } else if (arg.starts_with("--cpu-difficulty=")) {
            value = arg.substr(sizeof("--cpu-difficulty=") - 1);
            if (!parse_cpu_difficulty(value, State.cpuDifficulty)) {
                std::fprintf(stderr, "Invalid CPU difficulty '%.*s'. Use easy, medium, hard, expert, or 0-3.\n",
                    static_cast<int>(value.size()), value.data());
                return false;
            }
            continue;
        } else {
            continue;
        }

        int minigameNumber = -1;
        int minigameOverlay = DLL_NONE;
        if (!parse_minigame(value, minigameNumber, minigameOverlay)) {
            std::fprintf(stderr, "Invalid minigame '%.*s'. Use an installed Mario Party 4 minigame like 401 or m401.\n",
                static_cast<int>(value.size()), value.data());
            return false;
        }

        State.minigameLaunchEnabled = true;
        State.minigameNumber = minigameNumber;
        State.minigameOverlay = minigameOverlay;
    }

    return true;
}

} // namespace partyboard

extern "C" bool partyboard_cli_minigame_enabled(void)
{
    return State.minigameLaunchEnabled;
}

extern "C" int partyboard_cli_minigame_overlay(void)
{
    return State.minigameOverlay;
}

extern "C" int partyboard_cli_minigame_number(void)
{
    return State.minigameNumber;
}

extern "C" int partyboard_cli_cpu_difficulty(void)
{
    return State.cpuDifficulty;
}
