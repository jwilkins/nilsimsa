#!/usr/bin/env ruby
require 'mkmf'
require 'rbconfig'

CONFIG["DLEXT"] = "bundle"
CONFIG["LDSHARED"] = "$(CC) -shared"
CONFIG["CCDLFLAGS"] = " -fPIC"
        
have_header('ruby.h') or missing('ruby.h')

create_makefile( 'nilsimsa_native' )
