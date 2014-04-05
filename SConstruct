#!/usr/bin/python

#compiler = 'gcc'
compiler = 'clang'

ccflags = [
	'-IInclude',
	'-IThirdParty' ]

DefaultEnvironment( CC=compiler, CCFLAGS=ccflags )

VariantDir( 'Build/Source', 'Source' )
duck = Object( Glob( 'Build/Source/*.c' ) )

VariantDir( 'Build/ThirdParty/icu', 'ThirdParty/icu' )
icu = Object( Glob( 'Build/ThirdParty/icu/*.c' ) )

StaticLibrary( 'Build/Duck', [duck, icu] )

