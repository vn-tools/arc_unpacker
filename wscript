# vi: ft=python
from waflib import Logs
import os
from collections import OrderedDict

APPNAME = 'arc_unpacker'
try:
    VERSION = os.popen('git describe --tags').read().strip()
    VERSION_LONG = os.popen('git describe --always --dirty --long --tags').read().strip()
except:
    VERSION = '0.0'
    VERSION_LONG = '?'

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
        '-Wold-style-cast',
        '-ffloat-store',
        '-std=c++14']

    if ctx.options.debug:
        ctx.env.CXXFLAGS += ['-ggdb']
        Logs.info('Debug information enabled')
    else:
        ctx.env.CXXFLAGS += ['-s', '-O3']
        Logs.info('Debug information disabled, pass -d to enable')

    ctx.load('compiler_cxx')

    if ctx.env.DEST_OS in ['win32']:
        ctx.env.LINKFLAGS_UNICODE = ['-municode']

def configure_packages(ctx):
    ctx.check_cxx(
        lib = 'png',
        uselib_store = 'LIBPNG',
        mandatory = True)

    ctx.check_cxx(
        lib = 'jpeg',
        uselib_store = 'LIBJPEG',
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
            global_define = True,
            mandatory = False)

        ctx.check_cxx(
            lib = ['iconv'],
            header_name = 'iconv.h',
            uselib_store = 'LIBICONV',
            global_define = True,
            mandatory = True)
    else:
        ctx.check_cxx(
            lib = ['crypto'],
            header_name = 'openssl/rsa.h',
            uselib_store = 'LIBOPENSSL',
            global_define = True,
            mandatory = False)

    ctx.check_cxx(header_name = 'windows.h', global_define = True, mandatory = False)

    boost_found = False
    for suffix in ['', '-mt']:
        try:
            ctx.check_cxx(
                lib = ['boost_filesystem' + suffix, 'boost_system' + suffix],
                header_name = 'boost/filesystem.hpp',
                uselib_store = 'LIBBOOST_FILESYSTEM',
                mandatory = True,
                global_define = True,
                msg = 'Checking for boost_filesystem' + suffix)

            ctx.check_cxx(
                lib = ['boost_locale' + suffix],
                header_name = 'boost/locale.hpp',
                uselib_store = 'LIBBOOST_LOCALE',
                mandatory = True,
                global_define = True,
                msg = 'Checking for boost_locale' + suffix)

            boost_found = True
            break
        except ctx.errors.ConfigurationError as e:
            ctx.to_log('boost%s not found, trying another' % suffix)
    if not boost_found:
        ctx.fatal('Boost not found')

def configure(ctx):
    configure_flags(ctx)
    configure_packages(ctx)

def build(bld):
    common_sources = bld.path.ant_glob('src/**/*.cc')
    common_sources = [f for f in common_sources if f.name != 'main.cc']

    deps = OrderedDict()
    deps['_openssl'] = bld.is_defined('HAVE_OPENSSL_RSA_H')
    deps['_ansi'] = not bld.is_defined('HAVE_WINDOWS_H') or bld.env.DEST_OS == 'cygwin'
    deps['_win'] = bld.is_defined('HAVE_WINDOWS_H')
    deps['_dummy'] = True
    common_sources = _filter_deps(common_sources, bld, deps)

    path_to_src = bld.path.find_node('src').abspath()
    path_to_tests = bld.path.find_node('tests').abspath()

    program_sources = [bld.path.find_node('src/main.cc')]
    tests_sources = bld.path.ant_glob('tests/**/*.cc')

    bld.objects(
        source = common_sources,
        target = 'common',
        cxxflags = ['-iquote', path_to_src],
        includes = ['src'],
        use = [
            'LIBICONV',
            'LIBPNG',
            'LIBJPEG',
            'LIBZ',
            'LIBBOOST_FILESYSTEM',
            'LIBBOOST_LOCALE',
            'LIBOPENSSL',
        ])

    bld.program(
        source = program_sources,
        target = 'arc_unpacker',
        cxxflags = ['-iquote', path_to_src],
        defines = [ 'AU_VERSION="' + VERSION_LONG + '"' ],
        includes = ['src'],
        use = [
            'common',
            'UNICODE',
        ])

    bld.program(
        source = tests_sources,
        target = 'run_tests',
        cxxflags = [
            '-iquote', path_to_src,
            '-iquote', path_to_tests,
        ],
        includes = ['src', 'tests'],
        use = [ 'common' ])

# each file that depends on something that might or might not be there,
# contains fitting _INFIX in its name. (*_dummy files are expected to always
# contain fallback implementation.) This function is responsible for telling
# which implementation for each group should be compiled.
def _filter_deps(sources, bld, deps):
    def _filter_group(group, deps):
        for infix, enabled in deps.items():
            if enabled:
                for group_item in group:
                    file, file_infix = group_item
                    if file_infix == infix:
                        return file
        raise AssertionError('Can\'t figure out what to compile for %s' % group)
    default_files = []
    groups = {}
    for file in sources:
        group_key = file.relpath()
        file_infix = None
        for infix in deps.keys():
            if infix in group_key:
                file_infix = infix
                group_key = group_key.replace(infix, '')
                break
        if file_infix is None:
            default_files.append(file)
        else:
            if group_key not in groups:
                groups[group_key] = []
            groups[group_key].append([file, file_infix])
    chosen_files = [_filter_group(group, deps) for group in groups.values()]
    return default_files + chosen_files

def dist(ctx):
    ctx.algo  = 'zip'
    ctx.base_path = ctx.path
    ctx.excl = ctx.get_excl() + ' *.dll'

def distbin(ctx):
    from subprocess import call, PIPE
    from zipfile import ZipFile, ZIP_DEFLATED
    arch_name = 'arc_unpacker-' + VERSION + '-bin.zip'

    zip = ZipFile(arch_name, 'w', compression=ZIP_DEFLATED)
    for p in ctx.path.ant_glob('build/arc_unpacker.exe') + ctx.path.ant_glob('**/*.dll'):
        if not 'test' in p.name:
            target_name = p.name

            try:
                print('Stripping cruft from', target_name)
                call(['strip', '--strip-all', p.abspath()])
            except Exception as e:
                print(e)

            try:
                print('Compressing', target_name, 'with UPX')
                call(['upx', p.abspath()])
            except Exception as e:
                print(e)

            print('Adding', target_name)
            zip.write(p.abspath(), target_name, ZIP_DEFLATED)

    for p in ctx.path.ant_glob('extra/**/*'):
        target_name = os.path.relpath(p.abspath(), ctx.cur_script.parent.abspath())
        print('Adding', target_name)
        zip.write(p.abspath(), target_name, ZIP_DEFLATED)

    zip.close()

def test(ctx):
    ctx.exec_command('build/run_tests')
