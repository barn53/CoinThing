# -*- coding: utf-8 -*-

import tools
import os
import shutil

Import("env")
# Import("env", "projenv")
# print(env.Dump())
# print(projenv.Dump())


def preAction():

    print("###############################")
    print("#### preAction()")
    print("###############################")

    tools.writePreHeader(env)

    tools.writeSpiffsVersion(env)
    tools.prepareHTMLFiles(env)
    tools.removeBuildSpiffsFile(env)

    try:
        os.remove(env["PROJECT_DATA_DIR"] + "/wifi.json")
    except OSError:
        pass

    shutil.copyfile("secrets/wifi.json",
                    env["PROJECT_DATA_DIR"] + "/wifi.json")

    # maybe copy secrets.json


def postAction(source, target, env):

    print("###############################")
    print("#### postAction()")
    print("###############################")

    version = tools.getVersion()
    version = version.replace("*", "#")

    shutil.move(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/spiffs.bin",
                "assets/whaleticker_spiffs_" + version + "_wifi.bin")


preAction()

env.AddPostAction("$BUILD_DIR/spiffs.bin", postAction)
