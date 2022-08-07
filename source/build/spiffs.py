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

    tools.prepareSecretFiles(env, False)


def postAction(source, target, env):

    print("###############################")
    print("#### postAction()")
    print("###############################")

    version = tools.getVersion()
    version = version.replace("*", "#")

    shutil.move(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/spiffs.bin",
                "assets/whaleticker_spiffs_" + version + ".bin")


preAction()

env.AddPostAction("$BUILD_DIR/spiffs.bin", postAction)
