#!/usr/bin/env python
import Options
import os
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

def is_mingw (env):
	if 'CC' in env:
		cc = env['CC']
		if not isinstance (cc, str):
			cc = ''.join (cc)
		return cc.find ('mingw') != -1# or cc.find ('wine') != -1
	return False

def set_options(opt):
	opt.tool_options('compiler_cc')

def configure(conf):
	conf.check_tool('compiler_cc')
	conf.env['LMINGW'] = ''
	if is_mingw(conf.env):
		if not 'AR' in os.environ and not 'RANLIB' in os.environ:
			conf.env['AR'] = os.environ['CC'][:-3] + 'ar'
		Options.platform = 'win32'
		conf.env['program_PATTERN'] = '%s.exe'
		conf.env.append_value('CCFLAGS','-mms-bitfields')
		conf.env.append_value ('CCFLAGS', '-mwindows')
		conf.env.append_value('CCFLAGS','-DWINDOWS')
		conf.env["LMINGW"] = 'mingw32'
	else:
		# No speech synth for windows
		# No speech synth for *nix as well though
		#conf.check(header_name='espeak/espeak_lib.h')
		#conf.env.append_value('CCFLAGS','-DTTS')
		pass
	conf.check_cfg(package='glib-2.0',uselib_store = 'GLIB',mandatory = 1,args = '--cflags --libs')
	conf.check_cfg(path='sdl-config',package='',uselib_store = 'SDL',mandatory = 1,args = '--cflags --libs')
	conf.check_cfg(package='gtk+-2.0',uselib_store='GTK',mandatory=1,args='--cflags --libs')
	conf.check_cfg(path='xml2-config',package = '',uselib_store = 'LIBXML2',mandatory = 1,args = '--cflags --libs')
	conf.define('VERSION',VERSION)
	conf.env['VERSION'] = VERSION
	conf.write_config_header('config.h')

def build(bld):
	bld.add_subdirs('src')
