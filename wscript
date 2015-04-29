# vi: ft=python

APPNAME = 'arc_unpacker'
VERSION = '0.3'

def options(ctx):
	ctx.load('compiler_cxx')

def configure(ctx):
	#TODO: add mingw support
	ctx.load('compiler_cxx')

	ctx.env.CXXFLAGS = [
		'-Wall',
		'-Wextra',
		'-pedantic',
		'-Wwrite-strings',
		'-Wno-unused-variable',
		'-Wno-unused-parameter',
		'-std=c++11']

	ctx.check_cfg(
		package = 'libpng',
		args = ['--cflags', '--libs'],
		atleast_version = '1.4.0',
		uselib_store = 'LIBPNG',
		mandatory = True)

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

	ctx.check_cfg(
		package = 'zlib',
		args = ['--cflags', '--libs'],
		uselib_store = 'LIBZ',
		mandatory = True)

	ctx.check_cfg(
		package = 'libcrypto',
		args = ['--cflags', '--libs'],
		uselib_store = 'LIBOPENSSL',
		mandatory = True) #TODO: OpenSSL shouldn't be mandatory

	#TODO: what about iconv on Cygwin?

def build(ctx):
	ctx.program(
		source = ctx.path.ant_glob('src/**/*.cc'),
		target = 'arc_unpacker',
		cxxflags = ['-iquote', ctx.path.find_node('src').abspath()],
		use = [
			'LIBPNG',
			'LIBZ',
			'LIBBOOST_FILESYSTEM',
			'LIBBOOST_LOCALE',
			'LIBOPENSSL',
		],
	)

def dist(ctx):
	ctx.algo = 'zip'
	#TODO: strip executable
	ctx.files = ctx.path.ant_glob('build')

def test(ctx):
	#TODO
	raise RuntimeError('Not implemented')
