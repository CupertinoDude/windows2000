#ifndef _DBUTIL_H_
//***   dbutil.h -- debug helper routines
//

// dbutil.cpp
struct DBstkback {
    int fp;     // frame ptr
    int ret;    // return addr
};

int DBGetStackBack(int *pfp, struct DBstkback *pstkback, int nstkback);

// dump.c
// ...

#define _DBUTIL_H_
#endif // _DBUTIL_H_
