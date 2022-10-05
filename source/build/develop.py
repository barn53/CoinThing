# -*- coding: utf-8 -*-

import tools
import os
import shutil

Import("env")
# Import("env", "projenv")
# print(env.Dump())
# print(projenv.Dump())

withWiFi = True
withPipedream = False
withSettings = True


def preAction():

    print("###############################")
    print("#### preAction()")
    print("###############################")

    tools.writePreHeader(env)

    tools.writeSpiffsVersion(env)
    tools.prepareHTMLFiles(env)
    tools.removeBuildBinFiles(env)

    tools.createAssetsDirectory(withWiFi, withPipedream, withSettings)

    tools.prepareSecretFiles(env, withWiFi, withPipedream, withSettings)


preAction()
