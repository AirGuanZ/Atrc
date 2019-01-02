from math import exp
from scipy.optimize import fsolve

def InverseCDF(intervalCount = 1024):
    ret = []
    dem = 1.0 / (intervalCount - 1)
    for i in range(0, intervalCount):
        y = i * dem
        def equ(x):
            return 1 - 0.25 * exp(-x) - 0.75 * exp(-x / 3) - y
        ret.append(fsolve(equ, 0)[0])
    return ret

i = 0
for x in InverseCDF(2048):
    print("Real(%23.20f), " % x, end = "")
    i += 1
    if i % 6 == 0:
        print()
