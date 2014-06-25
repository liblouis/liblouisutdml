import os
import sys

# Some helper function definitions

def CheckPKGConfig(context, version):
  """Checks whether pkg-config of a given version can be found."""
  context.Message("Checking for pkg-config %s..." % version)
  ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
  context.Result(ret)
  return ret

def CheckPKG(context, name):
  """Checks for package using pkg-config"""
  context.Message("Checking for %s..." % name)
  ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
  context.Result(ret)
  return ret


packageConfigDefines = {'PACKAGE': 'liblouisutdml',
                    'PACKAGE_NAME': 'liblouisutdml',
                    'PACKAGE_STRING': 'liblouisutdml 2.5.0',
                    'PACKAGE_VERSION': '2.5.0',
                    'PACKAGE_BUGREPORT': 'john.boyer@abilitiessoft.com',
                    'PACKAGE_URL': '',
                    'VERSION': '2.5.0'
                   }
cSRCFiles = ['liblouisutdml/change_table.c',
             'liblouisutdml/makeContents.c',
             'liblouisutdml/examine_document.c',
             'liblouisutdml/liblouisutdml.c',
             'liblouisutdml/logging.c',
             'liblouisutdml/paths.c',
             'liblouisutdml/readconfig.c',
             'liblouisutdml/semantics.c',
             'liblouisutdml/transcribe_cdataSection.c',
             'liblouisutdml/transcribe_chemistry.c',
             'liblouisutdml/transcribe_computerCode.c',
             'liblouisutdml/transcribe_document.c',
             'liblouisutdml/transcribe_graphic.c',
             'liblouisutdml/transcribe_math.c',
             'liblouisutdml/transcribe_music.c',
             'liblouisutdml/transcribe_paragraph.c',
             'liblouisutdml/convert_utd.c',
             'liblouisutdml/utd2transinxml.c',
             'liblouisutdml/utd2dsbible.c',
             'liblouisutdml/utd2brf.c',
             'liblouisutdml/utd2pef.c',
             'liblouisutdml/utd2volumes.c',
             'liblouisutdml/transcriber.c',
            ]
jniSRCFiles = ['java/Jliblouisutdml.c']
toolsSRCFiles = ['tools/file2brl.c']
louisutdmlDepLibs = ['louis', 'xml2']
incDirs = ['liblouisutdml', 'gnulib']
libDirs = []

env = Environment()
conf = Configure(env,
                 custom_tests={'CheckPKGConfig': CheckPKGConfig,
                               'CheckPKG': CheckPKG},
                 config_h='liblouisutdml/config.h'
                )
if not conf.CheckCC():
  print('C compiler not found.')
  Exit(1)
if not conf.CheckPKGConfig('0.15.0'):
  if sys.platform == 'win32':
    liblouisIncDir = [os.path.join('..', 'liblouis', 'include')]
    libxml2IncDir = [os.path.join('..', 'libxml2', 'include', 'libxml2')]
    incDirs += libxml2IncDir + liblouisIncDir
    liblouisLibDir = [os.path.join('..', 'liblouis')]
    libxml2LibDir = [os.path.join('..', 'libxml2', 'lib')]
    libDirs += libxml2LibDir + liblouisLibDir
  else:
    print('pkg-config >= 0.15.0 not found.')
    Exit(1)
else:
  if not conf.CheckPKG('liblouis >= 2.5.4'):
    print('liblouis not found.')
    Exit(1)
  else:
    conf.env.ParseConfig('pkg-config --cflags --libs liblouis')
  if not conf.CheckPKG('libxml-2.0'):
    print('libxml2 not found.')
    Exit(1)
  else:
    conf.env.ParseConfig('pkg-config --cflags --libs libxml-2.0')
if not conf.CheckCHeader('stdlib.h'):
  print('stdlib.h not found.')
  Exit(1)
if not conf.CheckCHeader('string.h'):
  print('string.h not found.')
  Exit(1)
if not conf.CheckFunc('memset'):
  print('memset function not found.')
  Exit(1)
javaHome = os.environ.get('JAVA_HOME')
if javaHome:
  conf.env.Append(CPPPATH=[os.path.join(javaHome, 'include')])
if not conf.CheckCHeader('jni.h'):
  print('jni.h not found.')
  Exit(1)
else:
  incDirs += ['java']
  cSRCFiles += jniSRCFiles
for defName, defVal in packageConfigDefines.items():
  conf.Define(defName, '"%s"' % defVal)
env = conf.Finish()

# env.ParseConfig('pkg-config liblouis --cflags --libs')
# env.ParseConfig('pkg-config libxml-2.0 --cflags --libs')
env.Append(CPPDEFINES={'LIBLOUIS_TABLES_PATH': '\\"/usr/local/share/liblouis/tables/\\"',
                       'LBU_PATH': '\\"/usr/local/share/liblouisutdml/lbu_files/\\"'
                      },
           CPPPATH=incDirs,
           LIBPATH=libDirs)

utdmlSharedLibs = env.SharedLibrary('louisutdml', cSRCFiles, LIBS=louisutdmlDepLibs)
env.Program('file2brl', toolsSRCFiles + ['gnulib/progname.c', 'gnulib/version-etc.c'], LIBS=utdmlSharedLibs)

