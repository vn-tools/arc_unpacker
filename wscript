# vi: ft=python
import os

APPNAME = 'arc_unpacker'
VERSION = '0.3'

def options(ctx):
	ctx.load('compiler_cxx')

	ctx.add_option(
		'-c',
		'--cross-compile',
		dest = 'cross_compile',
		default = False,
		action = 'store_true',
		help = 'enable cross compiling using mingw-w64')

def find_mingw_dirs():
	MINGW_DIRS = ['~/mingw', '~/src/mingw', '/usr/i686-w64-mingw32']
	MINGW_DIRS = [os.path.expanduser(dir) for dir in MINGW_DIRS]
	dirs = []
	for dir in MINGW_DIRS:
		if os.path.isdir(dir):
			dirs.append(dir)
	if len(dirs) == 0:
		raise RuntimeError('No mingw folder found')
	return dirs

def configure_flags(ctx):
	ctx.env.CXXFLAGS = [
		'-Wall',
		'-Wextra',
		'-pedantic',
		'-Wwrite-strings',
		'-Wno-unused-variable',
		'-Wno-unused-parameter',
		'-std=c++11']

	if ctx.options.cross_compile:
		ctx.env.CXX = 'i686-w64-mingw32-g++'
		mingw_dirs = find_mingw_dirs()
		for dir in mingw_dirs:
			ctx.env.CXXFLAGS += ['-I' + os.path.join(dir, 'include')]
			ctx.env.LINKFLAGS += ['-L' + os.path.join(dir, 'lib')]
		ctx.env.LINKFLAGS += ['-lstdc++']
		ctx.env.LINKFLAGS_UNICODE = ['-municode']

	ctx.load('compiler_cxx')

def configure_packages(ctx):
	ctx.check_cxx(
		lib = ['png'],
		header_name = 'png.h',
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

	ctx.check_cxx(
		lib = ['z'],
		header_name = 'zlib.h',
		uselib_store = 'LIBZ',
		mandatory = True)

	ctx.check_cxx(
		lib = ['crypto'],
		header_name = 'openssl/rsa.h',
		uselib_store = 'LIBOPENSSL',
		mandatory = False)

	if ctx.options.cross_compile or ctx.env.DEST_OS == 'cygwin':
		ctx.check_cxx(
			lib = ['iconv'],
			header_name = 'iconv.h',
			uselib_store = 'LIBICONV',
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
