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


def postActionBuild(source, target, env):

    print("###############################")
    print("#### postActionBuild()")
    print("###############################")

    version = tools.getVersion()
    version = version.replace("*", "#")

    shutil.move(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/firmware.bin",
                "assets/whaleticker_" + version + ".bin")


def postActionSpiffs(source, target, env):

    print("###############################")
    print("#### postActionSpiffs()")
    print("###############################")

    version = tools.getVersion()
    version = version.replace("*", "#")

    shutil.move(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/spiffs.bin",
                "assets/whaleticker_spiffs_" + version + ".bin")


preAction()

env.AddPostAction("buildprog", postActionBuild)

env.AddPostAction("$BUILD_DIR/spiffs.bin", postActionSpiffs)
