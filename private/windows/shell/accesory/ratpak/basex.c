//-----------------------------------------------------------------------------
//  Package Title  ratpak                                                   
//  File           basex.c                                                    
//  Author         Timothy David Corrie Jr. (timc@microsoft.com)            
//  Copyright      (C) 1995-97 Microsoft                                    
//  Date           03-14-97                                                 
//                                                                          
//                                                                          
//  Description                                                             
//                                                                          
//     Contains number routines for internal base computations, these assume   
//  internal base is a power of 2.                                          
//                                                                          
//-----------------------------------------------------------------------------

#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ratpak.h>


// WARNING: This assumes return of a 64 bit entity is in edx:eax
// This assumption SHOULD always be true on X86
#pragma warning( disable : 4035 )
DWORDLONG __inline Mul32x32( IN DWORD a, IN DWORD b )

{
#ifdef _X86_ 
    __asm {
    mov eax, b
        mul a
        }
#else
    return (DWORDLONG)a * b;
#endif
}
#pragma warning( default : 4035 )

// Yeah well when the F__KING COMPILER gets a clue I'll change this back to 
// an inline (as opposed to the compiler looking at fastcall putting the args
// in registers, oh and then a) not making this inline, and b) pushing the 
// values anyway!

#ifdef _X86_ 
    #define Shr32xbase(x) \
            __asm { mov eax,DWORD PTR [x] } \
            __asm { mov edx,DWORD PTR [x+4] } \
            __asm { shrd eax,edx,BASEXPWR } \
            __asm { shr edx,BASEXPWR } \
            __asm { mov DWORD PTR [x],eax } \
            __asm { mov DWORD PTR [x+4],edx }
#else
    #define Shr32xbase(x) (x >>= BASEXPWR);
#endif



void _mulnumx( PNUMBER *pa, PNUMBER b );

//----------------------------------------------------------------------------
//
//    FUNCTION: mulnumx
//
//    ARGUMENTS: pointer to a number and a second number, the
//               base is always BASEX.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa *= b.
//    This is a stub which prevents multiplication by 1, this is a big speed
//    improvement.
//
//----------------------------------------------------------------------------

void __inline mulnumx( PNUMBER *pa, PNUMBER b )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {
        // If b is not one we multiply
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 1 || (*pa)->exp != 0 )
            { 
            // pa and b are both nonone.
            _mulnumx( pa, b );
            }
        else
            {
            // if pa is one and b isn't just copy b. and adjust the sign.
            long sign = (*pa)->sign;
            DUPNUM(*pa,b);
            (*pa)->sign *= sign;
            }
        }
    else
        {
        // B is +/- 1, But we do have to set the sign.
        (*pa)->sign *= b->sign;
        }
}

//----------------------------------------------------------------------------
//
//    FUNCTION: _mulnumx
//
//    ARGUMENTS: pointer to a number and a second number, the
//               base is always BASEX.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa *= b.
//    Assumes the base is BASEX of both numbers.  This algorithm is the
//    same one you learned in gradeschool, except the base isn't 10 it's
//    BASEX.
//
//----------------------------------------------------------------------------

void _mulnumx( PNUMBER *pa, PNUMBER b )

{
    PNUMBER c=NULL;         // c will contain the result.
    PNUMBER a=NULL;         // a is the dereferenced number pointer from *pa
    MANTTYPE *ptra;         // ptra is a pointer to the mantissa of a.
    MANTTYPE *ptrb;         // ptrb is a pointer to the mantissa of b.
    MANTTYPE *ptrc;         // ptrc is a pointer to the mantissa of c.
    MANTTYPE *ptrcoffset;   // ptrcoffset, is the anchor location of the next
                            // single digit multiply partial result.
    long iadigit=0;         // Index of digit being used in the first number.
    long ibdigit=0;         // Index of digit being used in the second number.
    MANTTYPE      da=0;     // da is the digit from the fist number.
    TWO_MANTTYPE  cy=0;     // cy is the carry resulting from the addition of
                            // a multiplied row into the result.
    TWO_MANTTYPE  mcy=0;    // mcy is the resultant from a single 
                            // multiply, AND the carry of that multiply.
    long  icdigit=0;        // Index of digit being calculated in final result.

    a=*pa;

    ibdigit = a->cdigit + b->cdigit - 1;
    createnum( c,  ibdigit + 1 );
    c->cdigit = ibdigit;
    c->sign = a->sign * b->sign;

    c->exp = a->exp + b->exp;
    ptra = MANT(a);
    ptrcoffset = MANT(c);

    for (  iadigit = a->cdigit; iadigit > 0; iadigit-- )
        {
        da =  *ptra++;
        ptrb = MANT(b);
        
        // Shift ptrc, and ptrcoffset, one for each digit 
        ptrc = ptrcoffset++;

        for ( ibdigit = b->cdigit; ibdigit > 0; ibdigit-- )
            {
            cy = 0;
            mcy = Mul32x32( da, *ptrb );
            if ( mcy )
                {
                icdigit = 0;
                if ( ibdigit == 1 && iadigit == 1 )
                    {
                    c->cdigit++;
                    }
                }
            // If result is nonzero, or while result of carry is nonzero...
            while ( mcy || cy )
                {
                
                // update carry from addition(s) and multiply.
                cy += (TWO_MANTTYPE)ptrc[icdigit]+((DWORD)mcy&((DWORD)~BASEX));
                
                // update result digit from 
                ptrc[icdigit++]=(MANTTYPE)((DWORD)cy&((DWORD)~BASEX));
                
                // update carries from
                Shr32xbase( mcy );
                Shr32xbase( cy );
                }
            *ptrb++;
            *ptrc++;
            }
        }
    
    // prevent different kinds of zeros, by stripping leading duplicate zeroes.
    // digits are in order of increasing significance.
    while ( c->cdigit > 1 && MANT(c)[c->cdigit-1] == 0 )
        {
        c->cdigit--;
        }

    destroynum( *pa );
    *pa=c;
}
//-----------------------------------------------------------------------------
//
//    FUNCTION: numpowlongx
//
//    ARGUMENTS: root as number power as long
//               number.
//
//    RETURN: None root is changed.
//
//    DESCRIPTION: changes numeric representation of root to
//    root ** power. Assumes base BASEX
//    decomposes the exponent into it's sums of powers of 2, so on average
//    it will take n+n/2 multiplies where n is the highest on bit.
//
//-----------------------------------------------------------------------------

void numpowlongx( IN OUT PNUMBER *proot, IN long power )

{
    PNUMBER lret=NULL;

    lret = longtonum( 1, BASEX );

    // Once the power remaining is zero we are done.
    while ( power > 0 )
        {
        // If this bit in the power decomposition is on, multiply the result
        // by the root number.
        if ( power & 1 )
            {
            mulnumx( &lret, *proot );
            }

        // multiply the root number by itself to scale for the next bit (i.e.
        // square it.
        mulnumx( proot, *proot );

        // move the next bit of the power into place.
        power >>= 1;
        }
    destroynum( *proot );
    *proot=lret;
    
}

void _divnumx( PNUMBER *pa, PNUMBER b );

//----------------------------------------------------------------------------
//
//    FUNCTION: divnumx
//
//    ARGUMENTS: pointer to a number a second number.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa /= b.
//    Assumes nRadix is the internal nRadix representation.
//    This is a stub which prevents division by 1, this is a big speed
//    improvement.
//
//----------------------------------------------------------------------------

void __inline divnumx( PNUMBER *pa, PNUMBER b )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {
        // b is not one.
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 1 || (*pa)->exp != 0 )
            {
            // pa and b are both not one.
            _divnumx( pa, b );
            }
        else
            {
            // if pa is one and b is not one, just copy b, and adjust the sign.
            long sign = (*pa)->sign;
            DUPNUM(*pa,b);
            (*pa)->sign *= sign;
            }
        }
    else
        {
        // b is one so don't divide, but set the sign.
        (*pa)->sign *= b->sign;
        }
}

//----------------------------------------------------------------------------
//
//    FUNCTION: _divnumx
//
//    ARGUMENTS: pointer to a number a second number.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa /= b.
//    Assumes nRadix is the internal nRadix representation.
//
//----------------------------------------------------------------------------

void _divnumx( PNUMBER *pa, PNUMBER b )

{
    PNUMBER a=NULL;         // a is the dereferenced number pointer from *pa
    PNUMBER c=NULL;         // c will contain the result.
    PNUMBER lasttmp = NULL; // lasttmp allows a backup when the algorithm
                            // guesses one bit too far.
    PNUMBER tmp = NULL;     // current guess being worked on for divide.
    PNUMBER rem = NULL;     // remainder after applying guess.
    long cdigits;           // count of digits for answer.
    MANTTYPE *ptrc;         // ptrc is a pointer to the mantissa of c.

    long thismax = maxout+ratio; // set a maximum number of internal digits
                                 // to shoot for in the divide.

    a=*pa;
    if ( thismax < a->cdigit )
        {
        // a has more digits than precision specified, bump up digits to shoot 
        // for.
        thismax = a->cdigit;
        }

    if ( thismax < b->cdigit )
        {
        // b has more digits than precision specified, bump up digits to shoot 
        // for.
        thismax = b->cdigit;
        }

    // Create c (the divide answer) and set up exponent and sign.
    createnum( c, thismax + 1 );
    c->exp = (a->cdigit+a->exp) - (b->cdigit+b->exp) + 1;
    c->sign = a->sign * b->sign;

    ptrc = MANT(c) + thismax;
    cdigits = 0;

    DUPNUM( rem, a );
    rem->sign = b->sign;
    rem->exp = b->cdigit + b->exp - rem->cdigit;

    while ( cdigits++ < thismax && !zernum(rem) )
        {
        long digit = 0;
        *ptrc = 0;
        while ( !lessnum( rem, b ) )
            {
            digit = 1;
            DUPNUM( tmp, b );
            destroynum( lasttmp );
            lasttmp=longtonum( 0, BASEX );
            while ( lessnum( tmp, rem ) )
                {
                destroynum( lasttmp );
                DUPNUM(lasttmp,tmp);
                addnum( &tmp, tmp, BASEX );
                digit *= 2;
                }
            if ( lessnum( rem, tmp ) )
                {    
                // too far, back up...
                destroynum( tmp );
                digit /= 2;
                tmp=lasttmp;
                lasttmp=NULL;
                }

            tmp->sign *= -1;
            addnum( &rem, tmp, BASEX ); 
            destroynum( tmp );
            destroynum( lasttmp );
            *ptrc |= digit;
            }
        rem->exp++;
        ptrc--;
        }
    cdigits--;
    if ( MANT(c) != ++ptrc )
        {
        memcpy( MANT(c), ptrc, (int)(cdigits*sizeof(MANTTYPE)) );
        }

    if ( !cdigits )
        {   
        // A zero, make sure no wierd exponents creep in
        c->exp = 0;
        c->cdigit = 1;
        }
    else
        {
        c->cdigit = cdigits;
        c->exp -= cdigits;
        // prevent different kinds of zeros, by stripping leading duplicate 
        // zeroes. digits are in order of increasing significance.
        while ( c->cdigit > 1 && MANT(c)[c->cdigit-1] == 0 )
            {
            c->cdigit--;
            }
        }

    destroynum( rem );

    destroynum( *pa );
    *pa=c;
}


