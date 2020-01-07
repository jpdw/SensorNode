#!/opt/local/bin/python
# Generate build info

import time

def generate_build_info():
    path = "src/"
    filename = "build_info.h"

    filename_include_define = filename.upper().replace('.','_') + '_'

    ts = time.localtime()

    ## time =========================================
    # human readable representation
    ts_str = time.strftime("%H:%M:%S",ts)
    ts_str_short = time.strftime("%H%M%S",ts)

    # turned into an integer value and 3 8bit values
    # based on the string representation of HHMMSS
    ts_int = int(float(time.strftime("%H%M%S",ts)))

    ts_int8_hr = int(float(time.strftime("%H",ts)))
    ts_int8_mi = int(float(time.strftime("%M",ts)))
    ts_int8_se = int(float(time.strftime("%S",ts)))

    ## date =========================================
    # human readable representation
    ds_str = time.strftime("%Y:%m:%d",ts)
    ds_str_short = time.strftime("%y%m%d",ts)

    # turned into an integer value and 3 8 bit values
    # based on the string representation of YYMMDD
    ds_int = int(float(time.strftime("%y%m%d",ts)))

    ds_int8_yr = int(float(time.strftime("%y",ts)))
    ds_int8_mo = int(float(time.strftime("%m",ts)))
    ds_int8_da = int(float(time.strftime("%d",ts)))

    build_type = "DEV"
    build_number = 0

    f = open("%s/%s"%(path,filename),'w')
    f.write('// build info - autogenerated do not manually edit\n')
    f.write('//\n')
    f.write('//\n')
    f.write('#ifndef %s\n'%filename_include_define)
    f.write('#define %s\n'%filename_include_define)
    f.write('\n') 
    f.write('//\n')
    f.write('// Build number\n')
    f.write('//\n')
    f.write('\n') 
    f.write('// Provide an incremental integer build number')
    f.write('//\n')
    f.write('#define __BI__BUILD_NUMBER %d\n'%build_number)
    f.write('#define __BI__BUILD_NUMBER_STR "%s-%d"\n'%(build_type, build_number))
    f.write('\n') 
    f.write('//\n')
    f.write('// Build date & time stamp for %s %s\n'%(ds_str, ts_str))
    f.write('//\n')
    f.write('\n')
    f.write('// Provide an integer representation of the time in human-readable form\n') 
    f.write('#define __BI__TIMESTAMP_INT %d\n'%ts_int)
    f.write('\n') 
    f.write('// Provide a 3 8-bit representation of the time in human-readable form\n') 
    f.write('#define __BI__TIMESTAMP_INT8_HR %d\n'%ts_int8_hr)
    f.write('#define __BI__TIMESTAMP_INT8_MI %d\n'%ts_int8_mi)
    f.write('#define __BI__TIMESTAMP_INT8_SE %d\n'%ts_int8_se)
    f.write('\n') 
    f.write('// Provide a 6 byte string representation of the time\n') 
    f.write('#define __BI__TIMESTAMP_STR "%s"\n'%ts_str_short)
    f.write('\n') 
    f.write('\n') 
    f.write('// Provide an integer representation of the date in human-readable form\n') 
    f.write('#define __BI__DATESTAMP_INT %d\n'%ds_int)
    f.write('\n') 
    f.write('// Provide a 3 8-bit representation of the date in human-readable form\n') 
    f.write('#define __BI__DATESTAMP_INT8_YR %d\n'%ds_int8_yr)
    f.write('#define __BI__DATESTAMP_INT8_MO %d\n'%ds_int8_mo)
    f.write('#define __BI__DATESTAMP_INT8_DA %d\n'%ds_int8_da)
    f.write('\n') 
    f.write('// Provide a 6 byte string representation of the date\n') 
    f.write('#define __BI__DATESTAMP_STR "%s"\n'%ds_str_short)
    f.write('\n') 
    f.write('#define __BI__DATEANDTIMESTAMP_STR "%d-%d"'%(ds_int, ts_int));
    f.write('\n') 
    f.write('#endif /* %s */\n'%filename_include_define)
    f.write('\n') 
    f.close()

