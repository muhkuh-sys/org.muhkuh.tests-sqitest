# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2011 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#


import os.path

#----------------------------------------------------------------------------
#
# Select the platforms to build
#
atPickNetxForBuild_All = ['NETX4000_RELAXED', 'NETX500', 'NETX90_MPW', 'NETX56', 'NETX50', 'NETX10']
AddOption('--netx',
          dest='atPickNetxForBuild',
          type='choice',
          choices=atPickNetxForBuild_All,
          action='append',
          metavar='NETX',
          help='Select the platforms to build for.')
atPickNetxForBuild = GetOption('atPickNetxForBuild')
if atPickNetxForBuild is None:
    atPickNetxForBuild = atPickNetxForBuild_All
fBuildIsFull = True
for strNetx in atPickNetxForBuild_All:
    if strNetx not in atPickNetxForBuild:
        fBuildIsFull = False
        break


#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')


# Create a build environment for the ARM9 based netX chips.
env_arm9 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.7', 'asciidoc'])
if 'NETX500' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX500', ['arch=armv5te'])
if 'NETX56' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX56', ['arch=armv5te'])
if 'NETX50' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX50', ['arch=armv5te'])
if 'NETX10' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX10', ['arch=armv5te'])

# Create a build environment for the Cortex-R7 and Cortex-A9 based netX chips.
env_cortexR7 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
if 'NETX4000_RELAXED' in atPickNetxForBuild:
    env_cortexR7.CreateCompilerEnv('NETX4000_RELAXED', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
if 'NETX90_MPW' in atPickNetxForBuild:
    env_cortexM4.CreateCompilerEnv('NETX90_MPW', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')

#----------------------------------------------------------------------------



sources_netx90_mpw_sqi_test = """
	src/init_netx_test.S
	src/netx90_sqi_test.c
	src/boot_drv_sqi.c
	src/sqitest_header.c
"""

#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('targets/version/version.h', 'templates/version.h')
atEnv.DEFAULT.Version('targets/version/version.xsl', 'templates/version.xsl')


#----------------------------------------------------------------------------
#
# Create the compiler environments.
#
astrCommonIncludePaths = ['src', '#platform/src', '#platform/src/lib', 'targets/version']

if 'NETX90_MPW' in atPickNetxForBuild:
    env_netx90_mpw_default = atEnv.NETX90_MPW.Clone()
    env_netx90_mpw_default.Replace(LDFILE = File('src/netx90.ld'))
    env_netx90_mpw_default.Append(CPPPATH = astrCommonIncludePaths)

#----------------------------------------------------------------------------
#
# Provide a function to build a binary.
#
def build_elf_bin(strName, tEnv, strBuildPath, astrSources):
	# Get the platform library.
	tLibPlatform = tEnv['PLATFORM_LIBRARY']

	tSrc = tEnv.SetBuildPath(strBuildPath, 'src', astrSources)
	tElf = tEnv.Elf(os.path.join(strBuildPath, strName+'.elf'), tSrc + [tLibPlatform])
	tBin = tEnv.ObjCopy(os.path.join(strBuildPath, strName+'.bin'), tElf)

	return tElf,tBin



#----------------------------------------------------------------------------
#
# Build the netx 90 SQI test
#
if 'NETX90_MPW' in atPickNetxForBuild:
    env_netx90_mpw_sqitest = env_netx90_mpw_default.Clone()
    env_netx90_mpw_sqitest.Append(CPPDEFINES = [['CFG_DEBUGMSG', '0']])
    elf_netx90_mpw_sqitest,bin_netx90_mpw_sqitest = build_elf_bin('netx90_mpw_sqi_test', env_netx90_mpw_sqitest, 'targets/netx90_mpw_sqi_test', sources_netx90_mpw_sqi_test)

#----------------------------------------------------------------------------
#
# Generate the LUA scripts from the template.
# This extracts symbols and enumeration values from the ELF file and inserts
# them into the LUA script.
# The netX500 ELF file is used here as a source for no special reason. All of
# the symbols and values which are used in the template are the same in every
# ELF file in this project.
#
	
tEnv = env_netx90_mpw_sqitest
tElf = elf_netx90_mpw_sqitest
lua_sqitest = tEnv.GccSymbolTemplate('targets/lua/sqitest.lua', tElf, GCCSYMBOLTEMPLATE_TEMPLATE='templates/sqitest.lua')

if fBuildIsFull==True:

    #----------------------------------------------------------------------------
    #
    # Make a local demo installation.
    #
    atCopyFiles = {
        # Copy all binaries.
        'targets/testbench/netx/netx90_mpw_sqitest.bin':                   bin_netx90_mpw_sqitest,

        # Copy all LUA modules.
        'targets/testbench/lua/sqitest.lua':                               lua_sqitest,

        # Copy all LUA scripts.
        'targets/testbench/test_sqi_flash_nxhx90.lua':                     'lua/test_sqi_flash_nxhx90.lua'
    }

    for tDst, tSrc in atCopyFiles.iteritems():
        Command(tDst, tSrc, Copy("$TARGET", "$SOURCE"))
