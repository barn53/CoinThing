#pragma once

#include <Arduino.h>

#define FOR_UPDATE_FILE F("/forupdate")

#define FS_NO_GLOBALS
#include <FS.h>
using FS = fs::FS;
using File = fs::File;
