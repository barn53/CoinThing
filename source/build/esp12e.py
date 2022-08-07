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

    tools.prepareSecretFiles(env, True)


def postAction(source, target, env):

    print("###############################")
    print("#### postAction()")
    print("###############################")

    version = tools.getVersion()
    version = version.replace("*", "#")

    shutil.copyfile(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/firmware.bin",
                    "assets/whaleticker_" + version + ".bin")


preAction()

env.AddPostAction("buildprog", postAction)
