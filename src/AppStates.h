//
//  AppStates.h
//  LAFTest
//
//  Created by gameover on 9/01/14.
//
//

#ifndef _H_APPSTATES
#define _H_APPSTATES

enum {
    kAPPCONTROLLER_INIT = 0,
    kAPPCONTROLLER_LOAD,
    kAPPCONTROLLER_ANALYZE,
    kAPPCONTROLLER_PLAY
};

enum {
    kDEBUGVIEW_SHOWINFO = 0,
    kDEBUGVIEW_SHOWSTATES,
    kDEBUGVIEW_SHOWPROPS
};

enum {
    kANALYZEVIEW_SHOW = 0
};

enum {
    kPLAYCONTROLLER_INIT = 0,
    kPLAYCONTROLLER_MAKE,
    kPLAYCONTROLLER_PLAY,
    kPLAYCONTROLLER_STOP
};

enum {
    kAPPVIEW_SHOWWARP = 0,
};

#endif