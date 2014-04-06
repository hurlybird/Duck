#!/usr/bin/python

# Read some configuration from the command line
platform = ARGUMENTS.get( 'os', Platform() )
architecture = ARGUMENTS.get( 'arch', 'x86_64' )
compiler = ARGUMENTS.get( 'cc', 'clang' )
debug = int( ARGUMENTS.get( 'debug', 1 ) )

# Set up the compiler flags
ccflags = []

ccflags += ['-arch', architecture]
ccflags += ['-Wno-int-conversion']		# Implicit integer-to-pointer conversion

if debug:
	ccflags += ['-DDEBUG=1']
	
else:
	ccflags += ['-DNDEBUG=1']

# Set include paths
cpppath = [
	'Include',
	'ThirdParty',
	'ThirdParty/icu'
]

# Define the build environemnt
env = Environment( PLATFORM=platform, ARCH=architecture, CC=compiler, CCFLAGS=ccflags, CPPPATH=cpppath )

# Build inside a build/platform/architecture specific location
builddir = 'Build/'

if debug:
	builddir += 'debug/'
	
else:
	builddir += 'release/'

builddir += str( platform ) + '/' + architecture + '/'

# Gather source file locations
sourcedirs = [
	'Source', 
	'ThirdParty/icu' 
]

# Compile
objects = []

for sourcedir in sourcedirs:
	env.VariantDir( builddir + sourcedir, sourcedir )
	objects += env.Object( Glob( builddir + sourcedir + '/*.c' ) )

# Link
StaticLibrary( builddir + 'Duck', objects )

