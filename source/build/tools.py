# -*- coding: utf-8 -*-

import subprocess
import glob
import string
import os
import shutil
import random
import json


def getVersion(pathCompatible):
    version = subprocess.check_output(["git", "describe", "--tags", "--always", "--match", "v[0-9]*"]).strip()
    version = version.decode('utf-8')
    version = version.replace("-2inch", "")  # remove the -2inch from the tag
    changes = subprocess.check_output(["git", "status", "--porcelain"]).strip()
    changes = changes.decode('utf-8')

    if len(changes) > 0:
        version = version+'-*'

    if pathCompatible:
        version = version.replace("*", "#")
        version = version.replace(":", "#")
        version = version.replace("?", "#")
        version = version.replace("|", "#")
        version = version.replace("/", "#")
        version = version.replace("\\", "#")

    return version


def writePreHeader(env):
    f = open(env["PROJECT_SRC_DIR"] + "/pre.h", "w")
    letters = string.ascii_lowercase
    f.write('#pragma once\n')
    f.write('#define SECRET_AP_PASSWORD F("%s")\n' % (''.join(random.choice(letters) for i in range(8))))
    f.write('#define VERSION F("%s")\n' % (getVersion(False)))
    f.close()


def writeSpiffsVersion(env):
    f = open(env["PROJECT_DATA_DIR"] + "/spiffs.version", "w")
    f.write('%s' % (getVersion(False)))
    f.close()
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/spiffs.version", env["PROJECT_DATA_DIR"] + "/settings.version")
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/spiffs.version", env["PROJECT_DATA_DIR"] + "/about.version")
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/spiffs.version", env["PROJECT_DATA_DIR"] + "/admin.version")


def removeBuildBinFiles(env):
    try:
        os.remove(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/spiffs.bin")
    except OSError:
        pass
    try:
        os.remove(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/firmware.bin")
    except OSError:
        pass


def getAssetsDirectoryName():
    version = getVersion(True)
    path = "assets/" + version + "/"
    return path


def getFirmwareFilename():
    version = getVersion(True)
    return "cointhing_" + version + ".bin"


def getSpiffsFilename(withWiFi, withPipedream):
    version = getVersion(True)
    name = "spiffs_" + version
    if withWiFi:
        name += "_wifi"
    if withPipedream:
        name += "_pipedream"
    name += ".bin"
    return name


def getUploadScriptFilename(withWiFi, withPipedream, sh):
    version = getVersion(True)
    name = "cointhing_upload_" + version
    if withWiFi:
        name += "_wifi"
    if withPipedream:
        name += "_pipedream"
    if sh:
        name += ".sh"
    else:
        name += ".bat"
    return name


def createAssetsDirectory():
    path = getAssetsDirectoryName()
    try:
        os.mkdir(path)
    except:
        pass
    return path


def createUploadScript(env, withWiFi, withPipedream):
    assets = getAssetsDirectoryName()
    filename = assets + "/" + getUploadScriptFilename(withWiFi, withPipedream, False)
    firmware = getFirmwareFilename()
    spiffs = getSpiffsFilename(withWiFi, withPipedream)

    f = open(filename, "w")
    f.write("python -m esptool erase_flash\n")
    f.write("python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 921600 write_flash 0x0 .\{0} 0x200000 .\{1} \n\n".format(firmware, spiffs))
    f.write("pio device monitor -b 115200\n")
    f.close()

    filename = assets + "/" + getUploadScriptFilename(withWiFi, withPipedream, True)

    f = open(filename, "w")
    f.write("export PATH={0}\n".format(env["ENV"]["Path"]))
    f.write("python -m esptool erase_flash\n".format(env["PYTHONEXE"]))
    f.write("python -m esptool --before default_reset --after hard_reset --chip esp8266 --baud 921600 write_flash 0x0 .\{0} 0x200000 .\{1} \n\n".format(firmware, spiffs))
    f.write("pio device monitor -b 115200\n")
    f.close()
    os.chmod(filename, 0o777)


def moveFirmware(env):
    assets = getAssetsDirectoryName()

    shutil.move(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/firmware.bin", assets + "/" + getFirmwareFilename())


def copyFirmware(env):
    assets = getAssetsDirectoryName()

    shutil.copyfile(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/firmware.bin", assets + "/" + getFirmwareFilename())


def moveSpiffs(env, withWiFi, withPipedream):
    assets = getAssetsDirectoryName()

    shutil.move(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/spiffs.bin", assets + "/" + getSpiffsFilename(withWiFi, withPipedream))


def copySpiffs(env, withWiFi, withPipedream):
    assets = getAssetsDirectoryName()

    shutil.copyfile(env["PROJECT_BUILD_DIR"] + "/" + env["PIOENV"] + "/spiffs.bin", assets + "/" + getSpiffsFilename(withWiFi, withPipedream))


def prepareSecretFiles(env, withWiFi, withPipedream, withSettings):
    try:
        os.remove(env["PROJECT_DATA_DIR"] + "/secrets.json")
    except OSError:
        pass

    try:
        os.remove(env["PROJECT_DATA_DIR"] + "/settings.json")
    except OSError:
        pass

    with open("secrets/secrets.json", "r") as f:
        secrets = json.load(f)

    if not withWiFi:
        secrets.pop("ssid", None)
        secrets.pop("pwd", None)
    if not withPipedream:
        secrets.pop("pipedream", None)

    with open(env["PROJECT_DATA_DIR"] + "/secrets.json", "w") as f:
        json.dump(secrets, f, indent=2, separators=(',', ':'))

    # settings.json only if avaiable
    if withSettings:
        try:
            shutil.copyfile("secrets/settings.json", env["PROJECT_DATA_DIR"] + "/settings.json")
        except OSError:
            pass


def prepareHTMLFiles(env):
    fileList = glob.glob(env["PROJECT_DATA_DIR"] + "/*.gz")
    for filePath in fileList:
        try:
            os.remove(filePath)
        except OSError:
            print("Error while deleting file")

    # Add ./source/build to PATH for windows to execute gzip
    subprocess.call(["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/settings.html"])
    subprocess.call(["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/about.html"])
    subprocess.call(["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/admin.html"])
    subprocess.call(["gzip", "-k", "-f", env["PROJECT_DATA_DIR"] + "/../html/style.css"])

    os.replace(env["PROJECT_DATA_DIR"] + "/../html/settings.html.gz", env["PROJECT_DATA_DIR"] + "/settings.html.gz")
    os.replace(env["PROJECT_DATA_DIR"] + "/../html/about.html.gz", env["PROJECT_DATA_DIR"] + "/about.html.gz")
    os.replace(env["PROJECT_DATA_DIR"] + "/../html/admin.html.gz", env["PROJECT_DATA_DIR"] + "/admin.html.gz")
    os.replace(env["PROJECT_DATA_DIR"] + "/../html/style.css.gz", env["PROJECT_DATA_DIR"] + "/style.css.gz")


def copyHTMLFiles(env):
    assets = getAssetsDirectoryName()

    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/settings.html.gz", assets + "/settings.html.gz")
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/about.html.gz", assets + "/about.html.gz")
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/admin.html.gz", assets + "/admin.html.gz")

    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/settings.version", assets + "/settings.version")
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/about.version", assets + "/about.version")
    shutil.copyfile(env["PROJECT_DATA_DIR"] + "/admin.version", assets + "/admin.version")
