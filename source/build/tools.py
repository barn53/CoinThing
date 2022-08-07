# -*- coding: utf-8 -*-

import subprocess
import glob
import string
import os
import shutil
import random


def getVersion():
    version = subprocess.check_output(
        ["git", "describe", "--tags", "--always", "--match", "v[0-9]*"]).strip()
    version = version.decode('utf-8')
    version = version.replace("-2inch", "")  # remove the -2inch from the tag
    changes = subprocess.check_output(["git", "status", "--porcelain"]).strip()
    changes = changes.decode('utf-8')

    if len(changes) > 0:
        version = version+'-*'

    return version


def writePreHeader(env):
    f = open(env["PROJECT_SRC_DIR"] + "/pre.h", "w")

    letters = string.ascii_lowercase
    f.write('#pragma once\n')
    f.write('#define SECRET_AP_PASSWORD F("%s")\n' %
            (''.join(random.choice(letters) for i in range(8))))
    f.write('#define VERSION F("%s")\n' % (getVersion()))
    f.close()


def writeSpiffsVersion(env):
    f = open(env["PROJECT_DATA_DIR"] + "/version.spiffs", "w")
    f.write('%s' % (getVersion()))
    f.close()


def removeBuildSpiffsFile(env):
    try:
        os.remove(env["PROJECT_BUILD_DIR"] + "/" +
                  env["PIOENV"] + "/spiffs.bin")
    except OSError:
        pass


def prepareSecretFiles(env, withWiFi):
    try:
        os.remove(env["PROJECT_DATA_DIR"] + "/wifi.json")
    except OSError:
        pass

    if withWiFi:
        shutil.copyfile("secrets/wifi.json",
                        env["PROJECT_DATA_DIR"] + "/wifi.json")

    # maybe copy secrets.json
    # maybe copy secrets_wifi.json


def prepareHTMLFiles(env):
    fileList = glob.glob(env["PROJECT_DATA_DIR"] + "/*.gz")
    for filePath in fileList:
        try:
            os.remove(filePath)
        except OSError:
            print("Error while deleting file")

    # Add ./source/build to PATH for windows to execute gzip
    subprocess.call(
        ["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/settings.html"])
    subprocess.call(
        ["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/about.html"])
    subprocess.call(
        ["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/admin.html"])
    subprocess.call(
        ["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/style.css"])

    os.replace(env["PROJECT_DATA_DIR"] + "/../html/settings.html.gz",
               env["PROJECT_DATA_DIR"] + "/settings.html.gz")
    os.replace(env["PROJECT_DATA_DIR"] + "/../html/about.html.gz",
               env["PROJECT_DATA_DIR"] + "/about.html.gz")
    os.replace(env["PROJECT_DATA_DIR"] + "/../html/admin.html.gz",
               env["PROJECT_DATA_DIR"] + "/admin.html.gz")
    os.replace(env["PROJECT_DATA_DIR"] + "/../html/style.css.gz",
               env["PROJECT_DATA_DIR"] + "/style.css.gz")
