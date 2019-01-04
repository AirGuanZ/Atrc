# -*- coding: utf-8 -*-

from math import exp
from scipy.optimize import fsolve

'''
    用于计算cdf^{-1}的数值表格
    cdf定义见 http://graphics.pixar.com/library/ApproxBSSRDF/paper.pdf
'''

def InverseCDF(arrSize = 2048):
    '''
    设cdf(x) = 1 - 0.25 * exp(-x) - 0.75 * exp(-x / 3)
    返回一个大小为arrSize的list，其中包含了cdf^{-1}在[0, 1]上的间隔为1 / (arrSize - 1)的采样值
    '''
    ret = []
    dem = 1.0 / (arrSize - 1)
    for i in range(0, arrSize):
        y = i * dem
        def equ(x):
            return 1 - 0.25 * exp(-x) - 0.75 * exp(-x / 3) - y
        ret.append(fsolve(equ, 0)[0])
    return ret

i = 0
for x in InverseCDF():
    print("%23.20f, " % x, end = "")
    i += 1
    if i % 6 == 0:
        print()
