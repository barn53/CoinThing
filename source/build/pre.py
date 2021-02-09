import string
import random

Import("env")
#Import("env", "projenv")

# access to global construction environment
print(env)

# access to project construction environment
#print(projenv)

# Dump construction environments (for debug purpose)
#print(env.Dump())
#print(projenv.Dump())

f = open(env["PROJECTSRC_DIR"] + "/pre.h", "w")

letters = string.ascii_lowercase

f.write('#pragma once\n')
f.write('#define HOST_NAME "CoinThing-%s"\n'% (''.join(random.choice(letters) for i in range(6))))
f.write('#define SECRET_AP_PASSWORD "%s"\n'% (''.join(random.choice(letters) for i in range(6)))) 

f.close()

