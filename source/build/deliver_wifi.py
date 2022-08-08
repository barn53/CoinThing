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

    tools.prepareSecretFiles(env, True, False)

    tools.createAssetsDirectory()
    tools.createUploadScript(env, True)
    tools.copyHTMLFiles(env)


def postActionBuild(source, target, env):

    print("###############################")
    print("#### postActionBuild()")
    print("###############################")

    tools.moveFirmware(env)


def postActionSpiffs(source, target, env):

    print("###############################")
    print("#### postActionSpiffs()")
    print("###############################")

    tools.moveSpiffs(env, True)


preAction()

env.AddPostAction("buildprog", postActionBuild)

env.AddPostAction("$BUILD_DIR/spiffs.bin", postActionSpiffs)
