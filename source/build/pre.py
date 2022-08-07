# -*- coding: utf-8 -*-

import glob
import os
import random
import string
import subprocess

Import("env")
#Import("env", "projenv")

# access to global construction environment
print(env)

# access to project construction environment
# print(projenv)

# Dump construction environments (for debug purpose)
# print(env.Dump())
# print(projenv.Dump())

version = subprocess.check_output(
    ["git", "describe", "--tags", "--always", "--match", "v[0-9]*"]).strip()
version = version.decode('utf-8')
changes = subprocess.check_output(["git", "status", "--porcelain"]).strip()
changes = changes.decode('utf-8')

if len(changes) > 0:
    version = version+'-*'

f = open(env["PROJECT_SRC_DIR"] + "/pre.h", "w")

letters = string.ascii_lowercase

f.write('#pragma once\n')
f.write('#define SECRET_AP_PASSWORD F("%s")\n' %
        (''.join(random.choice(letters) for i in range(8))))

f.write('#define VERSION F("%s")\n' % (version))
f.close()


f = open(env["PROJECT_DATA_DIR"] + "/version.spiffs", "w")
f.write('%s' % (version))
f.close()


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
