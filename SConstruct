import os

configEnvDefines = {'PACKAGE': 'liblouisutdml',
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

env = Environment()
env.ParseConfig('pkg-config liblouis --cflags --libs')
env.ParseConfig('pkg-config libxml-2.0 --cflags --libs')
env.Append(CPPDEFINES={'LIBLOUIS_TABLES_PATH': '\\"/usr/local/share/liblouis/tables/\\"',
                       'LBU_PATH': '\\"/usr/local/share/liblouisutdml/lbu_files/\\"'
                      },
           CPPPATH=['liblouisutdml', 'java', 'gnulib'])
env.Append(**configEnvDefines)
javacDir = os.path.dirname(env.WhereIs('javac'))
if not javacDir:
  print("javac not found.")
  Exit(1)
javaBaseDir = os.path.join(javacDir, '..')
print("Java in:%s" % javaBaseDir)
conf = Configure(env, config_h='liblouisutdml/config.h')
if not conf.CheckCC():
  print("C compiler not found.")
  Exit(1)
if not conf.CheckCHeader('liblouis.h'):
  print("liblouis.h not found.")
  Exit(1)
for defName, defVal in configEnvDefines.items():
  conf.Define(defName, '"%s"' % defVal)
env = conf.Finish()

utdmlSharedLibs = env.SharedLibrary('louisutdml', cSRCFiles + jniSRCFiles, LIBS=louisutdmlDepLibs)
env.Program('file2brl', toolsSRCFiles + ['gnulib/progname.c', 'gnulib/version-etc.c'], LIBS=utdmlSharedLibs)

