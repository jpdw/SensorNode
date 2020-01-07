Import("env")
import imp

#env.Execute("pwd");
buildinfo = imp.load_source('generate_build_info','./tools/generate_build_info.py')

print "\nLoading extra_script_pre.py\n"
print "Current build targets are: ", map(str, BUILD_TARGETS)

def before_buildprog():
    print "Freshening version.h"
    # Re-write the version.h file with an incremented version
    # and the current date/time
    
    # read the current file
    buildinfo.generate_build_info()

#env.AddPreAction("buildprog", before_buildprog)

before_buildprog()

