// Credits: TwilitRealm

#ifndef PARTY_BOARD_CLI_H
#define PARTY_BOARD_CLI_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
namespace partyboard {

bool parseCommandLine(int argc, char* argv[]);

}

extern "C" {
#endif

bool partyboard_cli_minigame_enabled(void);
int partyboard_cli_minigame_overlay(void);
int partyboard_cli_minigame_number(void);
int partyboard_cli_cpu_difficulty(void);

#ifdef __cplusplus
}
#endif

#endif // PARTY_BOARD_CLI_H
