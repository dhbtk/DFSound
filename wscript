#!/usr/bin/env python
VERSION = '1.0.0'
try:
	git = Utils.cmd_output(['git','rev-parse','--short','HEAD'])
	if git:
		VERSION = (VERSION + '-' + git).strip()
except:
	pass
APPNAME = 'dfsound'

top = '.'
out = 'build'

def set_options(opt):
	opt.tool_options('compiler_cc')

def configure(conf):
	conf.check_tool('compiler_cc')
	conf.check_cfg(package='glib-2.0',uselib_store = 'GLIB',mandatory = 1,args = '--cflags --libs')
	conf.check_cfg(path='sdl-config',package='',uselib_store = 'SDL',mandatory = 1,args = '--cflags --libs')
	conf.check_cfg(package='gtk+-2.0',uselib_store='GTK',mandatory=1,args='--cflags --libs')
	conf.check_cfg(path='xml2-config',package = '',uselib_store = 'LIBXML2',mandatory = 1,args = '--cflags --libs')
	conf.define('VERSION',VERSION)
	conf.write_config_header('config.h')

def build(bld):
	bld.add_subdirs('src')