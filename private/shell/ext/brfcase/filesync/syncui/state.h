//
// state.h: Declares data, defines and struct types for user state code
//          module.
//
//

#ifndef __STATE_H__
#define __STATE_H__


/////////////////////////////////////////////////////  INCLUDES

/////////////////////////////////////////////////////  DEFINES

/////////////////////////////////////////////////////  MACROS

/////////////////////////////////////////////////////  TYPEDEFS

typedef struct tagUSERSTATE
    {
    }
    CState,  * LPState;


/////////////////////////////////////////////////////  EXPORTED DATA

/////////////////////////////////////////////////////  PUBLIC PROTOTYPES

#ifdef DEBUG

BOOL PUBLIC ProcessIniFile(void);
BOOL PUBLIC CommitIniFile(void);

#else

#define ProcessIniFile()
#define CommitIniFile()

#endif

#endif // __STATE_H__

