#!/usr/bin/python

import os
from os import path


# Read some configuration from the command line
platform = ARGUMENTS.get( 'os', Platform() )
architecture = ARGUMENTS.get( 'arch', 'x86_64' )
compiler = ARGUMENTS.get( 'cc', 'clang' )
debug = int( ARGUMENTS.get( 'debug', 1 ) )

# Set up the compiler flags
ccflags = []
ccflags += ['-arch', architecture]

if debug:
	ccflags += ['-DDEBUG=1']
	
else:
	ccflags += ['-DNDEBUG=1']

# Set include paths
cpppath = [
	'ThirdParty',
	'ThirdParty/icu'
]

# Define the build environemnt
env = Environment( PLATFORM=platform, ARCH=architecture, CC=compiler, CCFLAGS=ccflags, CPPPATH=cpppath )

# Build inside a build/platform/architecture specific location
builddir = 'Build'

if debug:
	builddir = path.join( builddir, 'Debug' )
	
else:
	builddir = path.join( builddir, 'Release' )

builddir = path.join( builddir, str( platform ), architecture )

# Gather source file locations
sourcedirs = [
	'Source', 
	path.join( 'ThirdParty', 'icu' )
]

headerdirs = [
	'.',
	'Source'
]

# Compile
objects = []

for sourcedir in sourcedirs:
	env.VariantDir( path.join( builddir, sourcedir ), sourcedir )
	objects += env.Object( Glob( path.join( builddir, sourcedir, '*.c' ) ) )

# Link
StaticLibrary( path.join( builddir, 'duck' ), objects )

# Copy public headers
publicdir = path.join( builddir, 'Include' )

for headerdir in headerdirs:
	headers = Glob( path.join( headerdir, '*.h' ) )
	for header in headers:
		src = str( header )
		dst = path.join( publicdir, path.basename( src ) )
		Command( dst, src, Copy( "$TARGET", "$SOURCE" ) )


