# -*- coding: utf-8 -*-

import tools
import os
import shutil

Import("env")
# Import("env", "projenv")
# print(env.Dump())
# print(projenv.Dump())

withWiFi = True
withPipedream = True
withSettings = False


def preAction():

    print("###############################")
    print("#### preAction()")
    print("###############################")

    tools.writePreHeader(env)

    tools.writeSpiffsVersion(env)
    tools.prepareHTMLFiles(env)
    tools.removeBuildBinFiles(env)

    tools.prepareSecretFiles(env, withWiFi, withPipedream, withSettings)

    tools.createAssetsDirectory()
    tools.createUploadScript(env, withWiFi, withPipedream)
    tools.copyHTMLFiles(env)


def postActionBuild(source, target, env):

    print("###############################")
    print("#### postActionBuild()")
    print("###############################")

    tools.copyFirmware(env)


def postActionSpiffs(source, target, env):

    print("###############################")
    print("#### postActionSpiffs()")
    print("###############################")

    tools.copySpiffs(env, withWiFi, withPipedream)


preAction()

env.AddPostAction("buildprog", postActionBuild)

env.AddPostAction("$BUILD_DIR/spiffs.bin", postActionSpiffs)
