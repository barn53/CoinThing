# -*- coding: utf-8 -*-

import os
import shutil
import tools

Import("env")

withWiFi = 1  # False: no wifi, 0: ssids[0], ...
withPipedream = True
withSettings = False
colorset = 1


def preAction():

    print("###############################")
    print("#### preAction()")
    print("###############################")

    tools.writePreHeader(env)

    tools.writeSpiffsVersion(env)
    tools.prepareHTMLFiles(env)
    tools.removeBuildBinFiles(env)

    tools.createAssetsDirectory(env, withWiFi, withPipedream, withSettings)

    tools.prepareSecretFiles(
        env, withWiFi, withPipedream, withSettings, colorset)
    tools.createUploadScript(env, withWiFi, withPipedream, withSettings)
    tools.copyHTMLFiles(env, withWiFi, withPipedream, withSettings)


def postActionBuild(source, target, env):

    print("###############################")
    print("#### postActionBuild()")
    print("###############################")

    tools.copyFirmware(env, withWiFi, withPipedream, withSettings)


def postActionSpiffs(source, target, env):

    print("###############################")
    print("#### postActionSpiffs()")
    print("###############################")

    tools.copySpiffs(env, withWiFi, withPipedream, withSettings)


tools.checkFlags(withWiFi, withPipedream, withSettings, colorset)
preAction()

env.AddPostAction("buildprog", postActionBuild)

env.AddPostAction("$BUILD_DIR/spiffs.bin", postActionSpiffs)
