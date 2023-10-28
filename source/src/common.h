#pragma once

#include <Arduino.h>

#define SECRETS_FILE F("/secrets.json")
#define FOR_UPDATE_FILE F("/forupdate")
#define VERSION_BEFORE_UPDATE_FILE F("/versionbeforeupdate")
#define COLOR_SET_FILE F("/colorset")
#define FAKE_GECKO_SERVER_FILE F("/fakegeckoserver")
#define POWERUP_SEQUENCE_COUNTER_FILE F("/powerupsequence")
#define POWERUP_SEQUENCE_COUNT_TO_RESET 4

#define FS_NO_GLOBALS
#include <FS.h>
using FS = fs::FS;
using File = fs::File;
using Dir = fs::Dir;
