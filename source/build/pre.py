# -*- coding: utf-8 -*-

import string
import random
import subprocess

Import("env")
#Import("env", "projenv")

# access to global construction environment
print(env)

# access to project construction environment
#print(projenv)

# Dump construction environments (for debug purpose)
#print(env.Dump())
#print(projenv.Dump())

version = subprocess.check_output(["git", "describe", "--tags", "--always"]).strip()
version = version.decode('utf-8')
changes = subprocess.check_output(["git", "status", "--porcelain"]).strip()
changes = changes.decode('utf-8')

if len(changes) > 0:
    version = version+'-*'

f = open(env["PROJECTSRC_DIR"] + "/pre.h", "w")

letters = string.ascii_lowercase

f.write('#pragma once\n')
f.write('#define HOST_NAME F("CoinThing-%s")\n'% (''.join(random.choice(letters) for i in range(6))))
f.write('#define SECRET_AP_PASSWORD F("%s")\n'% (''.join(random.choice(letters) for i in range(8))))

f.write('#define VERSION F("%s")\n'% (version))

f.close()

