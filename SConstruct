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
          help='Select the platforms to build the flasher for.')
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
env_arm9 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.7', 'asciidoc', 'exoraw-2.0.7_2'])
if 'NETX500' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX500', ['arch=armv5te'])
if 'NETX56' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX56', ['arch=armv5te'])
if 'NETX50' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX50', ['arch=armv5te'])
if 'NETX10' in atPickNetxForBuild:
    env_arm9.CreateCompilerEnv('NETX10', ['arch=armv5te'])

# Create a build environment for the Cortex-R7 and Cortex-A9 based netX chips.
env_cortexR7 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc', 'exoraw-2.0.7_2'])
if 'NETX4000_RELAXED' in atPickNetxForBuild:
    env_cortexR7.CreateCompilerEnv('NETX4000_RELAXED', ['arch=armv7', 'thumb'], ['arch=armv7-r', 'thumb'])

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc', 'exoraw-2.0.7_2'])
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
# Provide a function to build a flasher binary.
#
def flasher_build(strFlasherName, tEnv, strBuildPath, astrSources):
	# Get the platform library.
	tLibPlatform = tEnv['PLATFORM_LIBRARY']

	## Create the list of known SPI flashes.
	#srcSpiFlashes = tEnv.SPIFlashes(os.path.join(strBuildPath, 'spi_flash_types', 'spi_flash_types.c'), 'src/spi_flash_types.xml')
	#objSpiFlashes = tEnv.Object(os.path.join(strBuildPath, 'spi_flash_types', 'spi_flash_types.o'), srcSpiFlashes[0])
	## Extract the binary.
	#binSpiFlashes = tEnv.ObjCopy(os.path.join(strBuildPath, 'spi_flash_types', 'spi_flash_types.bin'), objSpiFlashes)
	## Pack the binary with exomizer.
	#exoSpiFlashes = tEnv.Exoraw(os.path.join(strBuildPath, 'spi_flash_types', 'spi_flash_types.exo'), binSpiFlashes)
	## Convert the packed binary to an object.
	#objExoSpiFlashes = tEnv.ObjImport(os.path.join(strBuildPath, 'spi_flash_types', 'spi_flash_types_exo.o'), exoSpiFlashes)
    #
	## Append the path to the SPI flash list.
	#tEnv.Append(CPPPATH = [os.path.join(strBuildPath, 'spi_flash_types')])

	tSrcFlasher = tEnv.SetBuildPath(strBuildPath, 'src', astrSources)
	tElfFlasher = tEnv.Elf(os.path.join(strBuildPath, strFlasherName+'.elf'), tSrcFlasher + [tLibPlatform])
	tBinFlasher = tEnv.ObjCopy(os.path.join(strBuildPath, strFlasherName+'.bin'), tElfFlasher)

	return tElfFlasher,tBinFlasher



#----------------------------------------------------------------------------
#
# Build the netx 90 SQI test
#
if 'NETX90_MPW' in atPickNetxForBuild:
    env_netx90_mpw_sqitest = env_netx90_mpw_default.Clone()
    env_netx90_mpw_sqitest.Append(CPPDEFINES = [['CFG_DEBUGMSG', '0']])
    elf_netx90_mpw_sqitest,bin_netx90_mpw_sqitest = flasher_build('netx90_mpw_sqi_test', env_netx90_mpw_sqitest, 'targets/netx90_mpw_sqi_test', sources_netx90_mpw_sqi_test)

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

#----------------------------------------------------------------------------
#
# Build the documentation.
#

#tDocSpiFlashTypesHtml = atEnv.DEFAULT.XSLT('targets/doc/spi_flash_types.html', ['src/spi_flash_types.xml', 'src/spi_flash_types_html.xsl'])
#tDocSpiFlashListTxt = atEnv.DEFAULT.XSLT('targets/doc/spi_flash_types.txt', ['src/spi_flash_types.xml', 'src/spi_flash_types_txt.xsl'])
#
#
## Get the default attributes.
#aAttribs = atEnv.DEFAULT['ASCIIDOC_ATTRIBUTES']
## Add some custom attributes.
#aAttribs.update(dict({
#    # Use ASCIIMath formulas.
#    'asciimath': True,
#
#    # Embed images into the HTML file as data URIs.
#    'data-uri': True,
#
#    # Use icons instead of text for markers and callouts.
#    'icons': True,
#
#    # Use numbers in the table of contents.
#    'numbered': True,
#
#    # Generate a scrollable table of contents on the left of the text.
#    'toc2': True,
#
#    # Use 4 levels in the table of contents.
#    'toclevels': 4
#}))
#
#doc = atEnv.DEFAULT.Asciidoc('targets/doc/flasher.html', 'doc/flasher.asciidoc', ASCIIDOC_BACKEND='html5', ASCIIDOC_ATTRIBUTES=aAttribs)


#----------------------------------------------------------------------------
#
# Build the artifact.
#
if fBuildIsFull==True:
#    strGroup = 'org.muhkuh.tools'
#    strModule = 'sqitest'
#
#    # Split the group by dots.
#    aGroup = strGroup.split('.')
#    # Build the path for all artifacts.
#    strModulePath = 'targets/jonchki/repository/%s/%s/%s' % ('/'.join(aGroup), strModule, PROJECT_VERSION)
#
#
#    strArtifact = 'lua5.1-flasher'
#
#    tArcList = atEnv.DEFAULT.ArchiveList('zip')
#
#    tArcList.AddFiles('netx/',
#        bin_netx4000_relaxed_nodbg,
#        bin_netx90_mpw_nodbg)
#		
#    tArcList.AddFiles('netx/debug/',
#        bin_netx4000_relaxed_dbg,
#        bin_netx90_mpw_dbg)
#		
#    tArcList.AddFiles('doc/',
#        doc,
#        tDocSpiFlashTypesHtml,
#        tDocSpiFlashListTxt)
#
#    tArcList.AddFiles('lua/',
#        lua_flasher,
#        'lua/flasher_test.lua')
#
#    tArcList.AddFiles('demo/',
#        'lua/cli_flash.lua',
#        'lua/demo_getBoardInfo.lua',
#        'lua/erase_complete_flash.lua',
#        'lua/erase_first_flash_sector.lua',
#        'lua/erase_first_flash_sector_intflash0.lua',
#        'lua/erase_first_flash_sector_intflash1.lua',
#        'lua/erase_first_flash_sector_intflash2.lua',
#        'lua/flash_intflash0.lua',
#        'lua/flash_intflash1.lua',
#        'lua/flash_intflash2.lua',
#        'lua/flash_parflash.lua',
#        'lua/flash_serflash.lua',
#        'lua/get_erase_areas_parflash.lua',
#        'lua/identify_intflash0.lua',
#        'lua/identify_parflash.lua',
#        'lua/identify_serflash.lua',
#        'lua/is_erased_parflash.lua',
#        'lua/read_bootimage.lua',
#        'lua/read_bootimage_intflash0.lua',
#        'lua/read_bootimage_intflash2.lua',
#        'lua/read_complete_flash.lua',
#        tDemoShowEraseAreas)
#
#    tArcList.AddFiles('',
#        'jonchki/%s.%s/install.lua' % (strGroup, strModule))
#
#    strBasePath = os.path.join(strModulePath, '%s-%s' % (strArtifact, PROJECT_VERSION))
#    tArtifactZip = atEnv.DEFAULT.Archive('%s.zip' % strBasePath, None, ARCHIVE_CONTENTS = tArcList)
#    tArtifactXml = atEnv.DEFAULT.Version('%s.xml' % strBasePath, 'jonchki/%s.%s/%s.xml' % (strGroup, strModule, strArtifact))
#    tArtifactPom = atEnv.DEFAULT.ArtifactVersion('%s.pom' % strBasePath, 'jonchki/%s.%s/pom.xml' % (strGroup, strModule))
#
#    # Create the SHA1 sums for the ZIP and XML.
#    atEnv.DEFAULT.Hash('%s.zip.sha1' % strBasePath, tArtifactZip)
#    atEnv.DEFAULT.Hash('%s.xml.sha1' % strBasePath, tArtifactXml)
#
#    #----------------------------------------------------------------------------
#    #
#    # Prepare the build folders for the other artifacts.
#    #
#    Command('targets/jonchki/flasher_cli/flasher_cli.lua',            'jonchki/org.muhkuh.tools.flasher_cli/flasher_cli.lua',            Copy("$TARGET", "$SOURCE"))
#    Command('targets/jonchki/flasher_cli/jonchkicfg.xml',             'jonchki/jonchkicfg.xml',                                          Copy("$TARGET", "$SOURCE"))
#    Command('targets/jonchki/flasher_cli/jonchkisys_windows_32.cfg',  'jonchki/org.muhkuh.tools.flasher_cli/jonchkisys_windows_32.cfg',  Copy("$TARGET", "$SOURCE"))
#    Command('targets/jonchki/flasher_cli/jonchkisys_windows_64.cfg',  'jonchki/org.muhkuh.tools.flasher_cli/jonchkisys_windows_64.cfg',  Copy("$TARGET", "$SOURCE"))
#
#    atEnv.DEFAULT.Version('targets/jonchki/flasher_cli/flasher_cli.xml', 'jonchki/org.muhkuh.tools.flasher_cli/flasher_cli.xml')
#
#
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
