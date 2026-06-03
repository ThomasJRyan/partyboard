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

struct MinigameOverlay {
    int number;
    int overlay;
};

constexpr MinigameOverlay Minigames[] = {
    {401, DLL_m401dll},
    {402, DLL_m402dll},
    {403, DLL_m403dll},
    {404, DLL_m404dll},
    {405, DLL_m405dll},
    {406, DLL_m406dll},
    {407, DLL_m407dll},
    {408, DLL_m408dll},
    {409, DLL_m409dll},
    {410, DLL_m410dll},
    {411, DLL_m411dll},
    {412, DLL_m412dll},
    {413, DLL_m413dll},
    {414, DLL_m414dll},
    {415, DLL_m415dll},
    {416, DLL_m416dll},
    {417, DLL_m417dll},
    {418, DLL_m418dll},
    {419, DLL_m419dll},
    {420, DLL_m420dll},
    {421, DLL_m421dll},
    {422, DLL_m422dll},
    {423, DLL_m423dll},
    {424, DLL_m424dll},
    {425, DLL_m425dll},
    {426, DLL_m426dll},
    {427, DLL_m427dll},
    {428, DLL_m428dll},
    {429, DLL_m429dll},
    {430, DLL_m430dll},
    {431, DLL_m431dll},
    {432, DLL_m432dll},
    {433, DLL_m433dll},
    {434, DLL_m434dll},
    {435, DLL_m435dll},
    {436, DLL_m436dll},
    {437, DLL_m437dll},
    {438, DLL_m438dll},
    {439, DLL_m439dll},
    {440, DLL_m440dll},
    {441, DLL_m441dll},
    {442, DLL_m442dll},
    {443, DLL_m443dll},
    {444, DLL_m444dll},
    {445, DLL_m445dll},
    {446, DLL_m446dll},
    {447, DLL_m447dll},
    {448, DLL_m448dll},
    {449, DLL_m449dll},
    {450, DLL_m450dll},
    {451, DLL_m451dll},
    {453, DLL_m453dll},
    {455, DLL_m455dll},
    {456, DLL_m456dll},
    {457, DLL_m457dll},
    {458, DLL_m458dll},
    {459, DLL_m459dll},
    {460, DLL_m460dll},
    {461, DLL_m461dll},
    {462, DLL_m462dll},
    {463, DLL_m463dll},
};

CliState State;

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

    for (const MinigameOverlay& minigame : Minigames) {
        if (minigame.number == parsedNumber) {
            number = minigame.number;
            overlay = minigame.overlay;
            return true;
        }
    }

    return false;
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

bool isCliMinigameLaunchEnabled()
{
    return State.minigameLaunchEnabled;
}

int cliMinigameNumber()
{
    return State.minigameNumber;
}

int cliCpuDifficulty()
{
    return State.cpuDifficulty;
}

} // namespace partyboard

extern "C" bool partyboard_cli_minigame_enabled(void)
{
    return partyboard::isCliMinigameLaunchEnabled();
}

extern "C" int partyboard_cli_minigame_overlay(void)
{
    return State.minigameOverlay;
}

extern "C" int partyboard_cli_minigame_number(void)
{
    return partyboard::cliMinigameNumber();
}

extern "C" int partyboard_cli_cpu_difficulty(void)
{
    return partyboard::cliCpuDifficulty();
}
