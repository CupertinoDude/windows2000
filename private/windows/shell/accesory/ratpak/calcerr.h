//
// CalcErr.h
//
// Defines the error codes thrown by ratpak and caught by Calculator
//
//
//  Ratpak errors are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-------+---------------------+-------------------------------+
//  |S|   R   |    Facility         |               Code            |
//  +-+-------+---------------------+-------------------------------+
//
//  where
//
//      S - Severity - indicates success/fail
//
//          0 - Success
//          1 - Fail
//
//      R - Reserved - not currently used for anything
//
//      r - reserved portion of the facility code. Reserved for internal
//              use. Used to indicate HRESULT values that are not status
//              values, but are instead message ids for display strings.
//
//      Facility - is the facility code
//
//      Code - is the actual error code
//
// This format is based losely on an OLE HRESULT and is compatible with the
// SUCCEEDED and FAILED marcos as well as the HRESULT_CODE macro

// CALC_E_DIVIDEBYZERO
//
// The current operation would require a divide by zero to complete
#define CALC_E_DIVIDEBYZERO     ((DWORD)0x80000000)

// CALC_E_DOMAIN
//
// The given input is not within the domain of this function
#define CALC_E_DOMAIN           ((DWORD)0x80000001)

// CALC_E_INDEFINITE
//
// The result of this function is undefined
#define CALC_E_INDEFINITE        ((DWORD)0x80000002)

// CALC_E_POSINFINITY
//
// The result of this function is Positive Infinity.
#define CALC_E_POSINFINITY      ((DWORD)0x80000003)

// CALC_E_NEGINFINITY
//
// The result of this function is Negative Infinity
#define CALC_E_NEGINFINITY      ((DWORD)0x80000004)

// CALC_E_ABORTED
//
// The user aborted the completion of this function
#define CALC_E_ABORTED          ((DWORD)0x80000005)

// CALC_E_INVALIDRANGE
//
// The given input is within the domain of the function but is beyond
// the range for which calc can successfully compute the answer
#define CALC_E_INVALIDRANGE     ((DWORD)0x80000006)

// CALC_E_OUTOFMEMORY
//
// There is not enough free memory to complete the requested function
#define CALC_E_OUTOFMEMORY      ((DWORD)0x80000007)

