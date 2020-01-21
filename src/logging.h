// include this for logging (production logging) and debugging (no production)
//

#include "buildConfig.h"

#ifdef INCLUDE_DEBUG
    #include <RemoteDebug.h>
    extern RemoteDebug Debug;
#endif

void mlog(const char*);
void mlog(String);
void setup_logging();
