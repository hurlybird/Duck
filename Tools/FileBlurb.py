#!/usr/bin/python3

import os
import sys
import argparse
import pathlib
import shutil
import hashlib
import re
import tempfile
import zlib

from pathlib import Path

def parse_command_line() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description='Write (or re-write) a source file blurb.'
    )
    
    parser.add_argument( '-c', '--copyright', action='store', default='Copyright N/A', help='Copyright String' )
    parser.add_argument( '-p', '--project', action='store', default='Project N/A', help='Project Name' )
    parser.add_argument( '-t', '--template', action='store', required=True, help='Blurb Template' )
    parser.add_argument( '-d', '--root-dir', dest='root_dir', action='store', default='./')
    parser.add_argument( 'files', nargs='+' )
    
    return parser


def write_blurb( file, filename, options ):
    name = os.path.split( filename )[1]
    
    with open( options.template, mode='r' ) as blurb:
        for line in blurb:
            line = line.replace( "$FILENAME", name )
            line = line.replace( "$PROJECT", options.project )
            line = line.replace( "$COPYRIGHT", options.copyright )
            file.write( line )
    

def main() -> None:
    args_parser = parse_command_line()
    options = args_parser.parse_args()

    files = []
    
    for file_pattern in options.files:
        for path in Path( options.root_dir ).rglob( file_pattern ):
            filename = str( path )
            files.append( filename )

    for filename in files:
        lines = []
        
        with open( filename, mode='r' ) as srcfile:
            lines = [line for line in srcfile]

        with open( filename, mode='w' ) as dstfile:
            write_blurb( dstfile, filename, options )
            
            end_of_blurb = False
            block_comment = False
            
            for line in lines:
                if not end_of_blurb:
                    if line.isspace():
                        pass
    
                    elif block_comment:
                        if line.rstrip().endswith( "*/" ):
                            block_comment = False
    
                    else:
                        if line.lstrip().startswith( "/*" ):
                            block_comment = True
                            
                        elif line.lstrip().startswith( "//" ):
                            pass
                            
                        else:
                            end_of_blurb = True

                if end_of_blurb:                
                    dstfile.write( line )

if __name__ == '__main__':
    main()




