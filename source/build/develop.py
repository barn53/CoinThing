# -*- coding: utf-8 -*-

import os
import shutil
import tools

Import("env")

withWiFi = 0  # False: no wifi, 0: ssids[0], ...
withPipedream = False
withSettings = True
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


tools.checkFlags(withWiFi, withPipedream, withSettings, colorset)
preAction()
