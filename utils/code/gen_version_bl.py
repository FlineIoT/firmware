#!/usr/bin/env python
import getopt
import os
import sys
import re
from optparse import OptionParser
from subprocess import Popen, PIPE
import subprocess

def main(argv):
    opts = None

    help = "\033[33m-v <bl_version>\033[0m"
    bl_version = ''
    path_version_h = ''
    path_version_ini = ''

    try:
        opts, args = getopt.getopt(argv, "v:i:hf:", ["version="])

    except getopt.GetoptError:
        print help

    if opts != None:
        for opt, arg in opts:
            if opt == '-h':
                print help
                sys.exit()
            elif opt in ("-v", "--version"):
                bl_version = arg
            elif opt in ("-i"):
                path_version_h = arg
            elif opt in ("-f"):
                path_version_ini = arg

    path, fl = os.path.split(os.path.realpath(__file__))
    script_path = os.path.join(path, '')

    #choose the version.ini path if passed in argument
    if path_version_ini == '':
        ini_file = open(script_path + "../../version.ini", 'r')
    else:
        ini_file = open(path_version_ini, 'r')

    #get major, minor, patch and bl versions from version.ini
    ini_file.readline()
    firmware_major = str(int(ini_file.readline()[6:]))
    firmware_minor = str(int(ini_file.readline()[6:]))
    firmware_patch = str(int(ini_file.readline()[6:]))
    hw_rev = ''

    if bl_version != '':
        dummy = str(int(ini_file.readline()[3:]))

        # Parse app version
        a = re.compile("[0-9]")
        if not a.match(bl_version):
            print "\033[31mERROR: Version must match the pattern bl_version\033[0m"
            sys.exit(0)
    else:
        bl_version = str(int(ini_file.readline()[3:]))

    hw_rev = str(int(ini_file.readline()[3:]))
    ini_file.close()

    #choose the version.ini path if passed in argument
    if path_version_ini == '':
        ini_file = open(script_path + "../../version.ini", 'w')
    else:
        ini_file = open(path_version_ini, 'w')
    #write major, minor, patch and bl version to version.ini
    ini_file.writelines("[version]" + "\nmajor=" + firmware_major + "\nminor=" + firmware_minor + "\npatch=" + firmware_patch + "\nbl=" + bl_version + "\nhw=" + hw_rev)
    ini_file.close()

    #choose the version.h path if passed in argument
    if path_version_h == '':
        header_file = open(script_path + "../../fline/bootloader/inc/version.h", 'w')
    else:
        header_file = open(path_version_h, 'w')

    #update version.h
    header_file.seek(0)

    header_file.write( "#ifndef __VERSION_H__\n")
    header_file.write( "#define __VERSION_H__\n")
    header_file.write( "\n")
    header_file.write( "/* clang-format off */")
    header_file.write( "\n")
    header_file.write( "#define BL_REV " + bl_version + "\n")
    header_file.write( "\n")
    header_file.write( "/* clang-format on */")
    header_file.write( "\n")
    header_file.write( "#endif // __VERSION_H\n")

    print "\033[32mVersion set to " + bl_version + ".\033[0m"
    print "\033[33mWARN: Please compile the sources from scratch.\033[0m"

if __name__ == '__main__':
    main(sys.argv[1:])
