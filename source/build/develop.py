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

    tools.prepareSecretFiles(env, True, False, True)


preAction()
