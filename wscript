# vi: ft=python
from waflib import Logs
import os

APPNAME = 'arc_unpacker'
VERSION = '0.3'

def options(ctx):
	ctx.load('compiler_cxx')

	ctx.add_option(
		'-d',
		'--debug',
		dest = 'debug',
		default = False,
		action = 'store_true',
		help = 'enable emitting debug information')

def configure_flags(ctx):
	ctx.env.CXXFLAGS = [
		'-Wall',
		'-Wextra',
		'-pedantic',
		'-Wwrite-strings',
		'-Wno-unused-variable',
		'-Wno-unused-parameter',
		'-std=c++11']

	if ctx.options.debug:
		ctx.env.CXXFLAGS += ['-g']
		Logs.info('Debug information enabled')
	else:
		Logs.info('Debug information disabled, pass -d to enable')

	ctx.load('compiler_cxx')

	if ctx.env.DEST_OS in ['win32', 'cygwin']:
		ctx.env.LINKFLAGS_UNICODE = ['-municode']

def configure_packages(ctx):
	ctx.check_cxx(
		lib = ['png'],
		header_name = 'png.h',
		uselib_store = 'LIBPNG',
		mandatory = True)

	ctx.check_cxx(
		lib = ['z'],
		header_name = 'zlib.h',
		uselib_store = 'LIBZ',
		mandatory = True)

	if ctx.env.DEST_OS in ['win32', 'cygwin']:
		ctx.check_cxx(
			lib = ['crypto', 'gdi32'],
			header_name = 'openssl/rsa.h',
			uselib_store = 'LIBOPENSSL',
			mandatory = False)

		ctx.check_cxx(
			lib = ['boost_filesystem-mt', 'boost_system-mt'],
			header_name = 'boost/filesystem.hpp',
			uselib_store = 'LIBBOOST_FILESYSTEM',
			mandatory = True)

		ctx.check_cxx(
			lib = ['boost_locale-mt'],
			header_name = 'boost/locale.hpp',
			uselib_store = 'LIBBOOST_LOCALE',
			mandatory = True)

		ctx.check_cxx(
			lib = ['iconv'],
			header_name = 'iconv.h',
			uselib_store = 'LIBICONV',
			mandatory = True)
	else:
		ctx.check_cxx(
			lib = ['crypto'],
			header_name = 'openssl/rsa.h',
			uselib_store = 'LIBOPENSSL',
			mandatory = False)

		ctx.check_cxx(
			lib = ['boost_filesystem', 'boost_system'],
			header_name = 'boost/filesystem.hpp',
			uselib_store = 'LIBBOOST_FILESYSTEM',
			mandatory = True)

		ctx.check_cxx(
			lib = ['boost_locale'],
			header_name = 'boost/locale.hpp',
			uselib_store = 'LIBBOOST_LOCALE',
			mandatory = True)

def configure(ctx):
	configure_flags(ctx)
	configure_packages(ctx)

def build(ctx):
	common_sources = ctx.path.ant_glob('src/**/*.cc')
	common_sources = [f for f in common_sources if f.name != 'main.cc']

	if not ctx.env.LIB_LIBOPENSSL:
		common_sources = [f for f in common_sources if 'tfpk_archive' not in f.name]

	path_to_src = ctx.path.find_node('src').abspath()
	path_to_tests = ctx.path.find_node('tests').abspath()

	program_sources = [ctx.path.find_node('src/main.cc')]
	tests_sources = ctx.path.ant_glob('tests/**/*.cc')

	ctx.objects(
		source = common_sources,
		target = 'common',
		cxxflags = ['-iquote', path_to_src],
		use = [
			'LIBICONV',
			'LIBPNG',
			'LIBZ',
			'LIBBOOST_FILESYSTEM',
			'LIBBOOST_LOCALE',
			'LIBOPENSSL',
		])

	ctx.program(
		source = program_sources,
		target = 'arc_unpacker',
		cxxflags = ['-iquote', path_to_src],
		use = [
			'common',
			'UNICODE',
		])

	ctx.program(
		source = tests_sources,
		target = 'run_tests',
		cxxflags = [
			'-iquote', path_to_src,
			'-iquote', path_to_tests,
		],
		use = [ 'common' ])

def dist(ctx):
	ctx.algo = 'zip'
	#TODO: strip executable
	ctx.files = ctx.path.ant_glob('build')

def test(ctx):
    ctx.exec_command('build/run_tests')
