# -*- coding: utf-8 -*-
"""
Created on Thu Nov  4 01:20:34 2021

@author: God
"""
Import("env")
# access to global build environment
import mmap
import os
def BeforeBuild():
    found = False
    with open(r"src\OTAWEBAP.cpp", "r") as f:   
        line = f.readline();
        if(line[:-1] == 'SetupServer();'):
                found = True;
        for line in f:
            line = line.strip();
            if(line == 'SetupServer();'):
                found = True;
        if(not found):
            raise Exception("OTA function not found, you need to use it to preserve OTA functions in your new deploy")
        else:
            print('function SetupServer was found')
    f.close()
env.AddPreAction("buildfs", BeforeBuild())
