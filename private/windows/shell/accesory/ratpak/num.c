//-----------------------------------------------------------------------------
//  Package Title  ratpak                                                   
//  File           num.c                                                    
//  Author         Timothy David Corrie Jr. (timc@microsoft.com)            
//  Copyright      (C) 1995-97 Microsoft                                    
//  Date           01-16-95                                                 
//                                                                          
//                                                                          
//  Description                                                             
//                                                                          
//     Contains number routines for add, mul, div, rem and other support       
//  and longs.                                                              
//                                                                          
//  Special Information                                                     
//                                                                          
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

//----------------------------------------------------------------------------
//
//    FUNCTION: addnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               nRadix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa += b.
//    Assumes nRadix is the base of both numbers.
//
//    ALGORITHM: Adds each digit from least significant to most
//    significant.
//
//
//----------------------------------------------------------------------------

void _addnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix );

void __inline addnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    if ( b->cdigit > 1 || b->mant[0] != 0 )
        {    // If b is zero we are done.
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 0 )
            { // pa and b are both nonzero.
            _addnum( pa, b, nRadix );
            }
        else
            { // if pa is zero and b isn't just copy b.
            DUPNUM(*pa,b);
            }
        }
}

void _addnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    PNUMBER c=NULL;     // c will contain the result.
    PNUMBER a=NULL;     // a is the dereferenced number pointer from *pa
    MANTTYPE *pcha;     // pcha is a pointer to the mantissa of a.
    MANTTYPE *pchb;     // pchb is a pointer to the mantissa of b.
    MANTTYPE *pchc;     // pchc is a pointer to the mantissa of c.
    long cdigits;       // cdigits is the max count of the digits results
                        // used as a counter.
    long mexp;          // mexp is the exponent of the result.
    MANTTYPE  da;       // da is a single 'digit' after possible padding.
    MANTTYPE  db;       // db is a single 'digit' after possible padding.
    MANTTYPE  cy=0;     // cy is the value of a carry after adding two 'digits'
    long  fcompla = 0;  // fcompla is a flag to signal a is negative.
    long  fcomplb = 0;  // fcomplb is a flag to signal b is negative.

    a=*pa;
    
    
    // Calculate the overlap of the numbers after alignment, this includes
    // necessary padding 0's
    cdigits = max( a->cdigit+a->exp, b->cdigit+b->exp ) -
            min( a->exp, b->exp );

    createnum( c, cdigits + 1 );
    c->exp = min( a->exp, b->exp );
    mexp = c->exp;
    c->cdigit = cdigits;
    pcha = MANT(a);
    pchb = MANT(b);
    pchc = MANT(c);
    
    // Figure out the sign of the numbers
    if ( a->sign != b->sign )
        {
        cy = 1;
        fcompla = ( a->sign == -1 );
        fcomplb = ( b->sign == -1 );
        }
    
    // Loop over all the digits, real and 0 padded. Here we know a and b are
    // aligned 
    for ( ;cdigits > 0; cdigits--, mexp++ )
        {
        
        // Get digit from a, taking padding into account.
        da = ( ( ( mexp >= a->exp ) && ( cdigits + a->exp - c->exp > 
                    (c->cdigit - a->cdigit) ) ) ? 
                    *pcha++ : 0 );
        // Get digit from b, taking padding into account.
        db = ( ( ( mexp >= b->exp ) && ( cdigits + b->exp - c->exp > 
                    (c->cdigit - b->cdigit) ) ) ? 
                    *pchb++ : 0 );
        
        // Handle complementing for a and b digit. Might be a better way, but
        // haven't found it yet.
        if ( fcompla )
            {
            da = (MANTTYPE)(nRadix) - 1 - da;
            }
        if ( fcomplb )
            {
            db = (MANTTYPE)(nRadix) - 1 - db;
            }
        
        // Update carry as necessary
        cy = da + db + cy;
        *pchc++ = (MANTTYPE)(cy % (MANTTYPE)nRadix);
        cy /= (MANTTYPE)nRadix;
        }
    
    // Handle carry from last sum as extra digit
    if ( cy && !(fcompla || fcomplb) )
        {
        *pchc++ = cy;
        c->cdigit++;
        }
    
    // Compute sign of result
    if ( !(fcompla || fcomplb) )
        {
        c->sign = a->sign;
        }
    else
        {
        if ( cy )
            {
            c->sign = 1;
            }
        else
            {
            // In this particular case an overflow or underflow has occoured 
            // and all the digits need to be complemented, at one time an 
            // attempt to handle this above was made, it turned out to be much 
            // slower on average.
            c->sign = -1;
            cy = 1;
            for ( ( cdigits = c->cdigit ), (pchc = MANT(c) ); 
                cdigits > 0; 
                cdigits-- )
                {
                cy = (MANTTYPE)nRadix - (MANTTYPE)1 - *pchc + cy;
                *pchc++ = (MANTTYPE)( cy % (MANTTYPE)nRadix );
                cy /= (MANTTYPE)nRadix;
                }
            }
        }
    
    // Remove leading zeroes, remember digits are in order of
    // increasing significance. i.e. 100 would be 0,0,1
    while ( c->cdigit > 1 && *(--pchc) == 0 )
        {
        c->cdigit--;
        }
    destroynum( *pa );
    *pa=c;
}

//----------------------------------------------------------------------------
//
//    FUNCTION: mulnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               nRadix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa *= b.
//    Assumes nRadix is the nRadix of both numbers.  This algorithm is the
//    same one you learned in gradeschool.
//
//----------------------------------------------------------------------------

void _mulnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix );

void __inline mulnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {    // If b is one we don't multiply exactly.
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 1 || (*pa)->exp != 0 )
            { // pa and b are both nonone.
            _mulnum( pa, b, nRadix );
            }
        else
            { // if pa is one and b isn't just copy b, and adjust the sign.
            long sign = (*pa)->sign;
            DUPNUM(*pa,b);
            (*pa)->sign *= sign;
            }
        }
    else
        {    // But we do have to set the sign.
        (*pa)->sign *= b->sign;
        }
}

void _mulnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    PNUMBER c=NULL;         // c will contain the result.
    PNUMBER a=NULL;         // a is the dereferenced number pointer from *pa
    MANTTYPE *pcha;         // pcha is a pointer to the mantissa of a.
    MANTTYPE *pchb;         // pchb is a pointer to the mantissa of b.
    MANTTYPE *pchc;         // pchc is a pointer to the mantissa of c.
    MANTTYPE *pchcoffset;   // pchcoffset, is the anchor location of the next
                            // single digit multiply partial result.
    long iadigit = 0;       // Index of digit being used in the first number.
    long ibdigit = 0;       // Index of digit being used in the second number.
    MANTTYPE  da = 0;       // da is the digit from the fist number.
    TWO_MANTTYPE  cy = 0;   // cy is the carry resulting from the addition of
                            // a multiplied row into the result.
    TWO_MANTTYPE  mcy = 0;  // mcy is the resultant from a single 
                            // multiply, AND the carry of that multiply.
    long  icdigit = 0;      // Index of digit being calculated in final result.

    a=*pa;
    ibdigit = a->cdigit + b->cdigit - 1;
    createnum( c,  ibdigit + 1 );
    c->cdigit = ibdigit;
    c->sign = a->sign * b->sign;

    c->exp = a->exp + b->exp;
    pcha = MANT(a);
    pchcoffset = MANT(c);

    for (  iadigit = a->cdigit; iadigit > 0; iadigit-- )
        {
        da =  *pcha++;
        pchb = MANT(b);
        
        // Shift pchc, and pchcoffset, one for each digit 
        pchc = pchcoffset++;

        for ( ibdigit = b->cdigit; ibdigit > 0; ibdigit-- )
            {
            cy = 0;
            mcy = (TWO_MANTTYPE)da * *pchb;
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
                cy += (TWO_MANTTYPE)pchc[icdigit]+(mcy%(TWO_MANTTYPE)nRadix);
                
                // update result digit from 
                pchc[icdigit++]=(MANTTYPE)(cy%(TWO_MANTTYPE)nRadix);
                
                // update carries from
                mcy /= (TWO_MANTTYPE)nRadix;
                cy /= (TWO_MANTTYPE)nRadix;
                }
            *pchb++;
            *pchc++;
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


//----------------------------------------------------------------------------
//
//    FUNCTION: remnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               nRadix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa %= b.
//            Repeatedly subtracts off powers of 2 of b until *pa < b.
//
//
//----------------------------------------------------------------------------

void remnum( PNUMBER *pa, PNUMBER b, long nRadix )

{
    PNUMBER tmp = NULL;     // tmp is the working remainder.
    PNUMBER lasttmp = NULL; // lasttmp is the last remainder which worked.
    
    // Once *pa is less than b, *pa is the remainder.
    while ( !lessnum( *pa, b ) && !fhalt )
        {
        DUPNUM( tmp, b );
        if ( lessnum( tmp, *pa ) )
            {
            // Start off close to the right answer for subtraction.
            tmp->exp = (*pa)->cdigit+(*pa)->exp - tmp->cdigit;
            if ( MSD(*pa) <= MSD(tmp) )
                {
                // Don't take the chance that the numbers are equal.
                tmp->exp--;
                }
            }

        destroynum( lasttmp );
        lasttmp=longtonum( 0, nRadix );

        while ( lessnum( tmp, *pa ) ) 
            {
            DUPNUM( lasttmp, tmp );
            addnum( &tmp, tmp, nRadix );
            }

        if ( lessnum( *pa, tmp ) )
            {    
            // too far, back up...
            destroynum( tmp );
            tmp=lasttmp;
            lasttmp=NULL;
            }
        
        // Subtract the working remainder from the remainder holder.
        tmp->sign = -1*(*pa)->sign;
        addnum( pa, tmp, nRadix ); 

        destroynum( tmp );
        destroynum( lasttmp );

        } 
}


//---------------------------------------------------------------------------
//
//    FUNCTION: divnum
//
//    ARGUMENTS: pointer to a number a second number, and the
//               nRadix.
//
//    RETURN: None, changes first pointer.
//
//    DESCRIPTION: Does the number equivalent of *pa /= b.
//    Assumes nRadix is the nRadix of both numbers.
//
//---------------------------------------------------------------------------

void _divnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix );

void __inline divnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {    
    	// b is not one
        _divnum( pa, b, nRadix );
        }
    else
        {    // But we do have to set the sign.
        (*pa)->sign *= b->sign;
        }
}

void _divnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    PNUMBER a = NULL;
    PNUMBER c = NULL;
    PNUMBER tmp = NULL;
    PNUMBER rem = NULL;
    PLINKEDLIST pll = NULL;
    PLINKEDLIST pllrover = NULL;
    long digit;
    long cdigits;
    BOOL bret;
    MANTTYPE *ptrc;
    long thismax = maxout+2;

    a=*pa;
    if ( thismax < a->cdigit )
        {
        thismax = a->cdigit;
        }

    if ( thismax < b->cdigit )
        {
        thismax = b->cdigit;
        }

    createnum( c, thismax + 1 );
    c->exp = (a->cdigit+a->exp) - (b->cdigit+b->exp) + 1;
    c->sign = a->sign * b->sign;

    ptrc = MANT(c) + thismax;
    cdigits = 0;
    DUPNUM( rem, a );
    DUPNUM( tmp, b );
    tmp->sign = a->sign;
    rem->exp = b->cdigit + b->exp - rem->cdigit;
    
    // Build a table of multiplications of the divisor, this is quicker for 
    // more than nRadix 'digits'
    pll = (PLINKEDLIST)zmalloc( sizeof( LINKEDLIST ) );
    pll->pnum = longtonum( 0L, nRadix );
    pll->llprev = NULL;
    for ( cdigits = 1; cdigits < (long)nRadix; cdigits++ )
        {
        pllrover = (PLINKEDLIST)zmalloc( sizeof( LINKEDLIST ) );
        pllrover->pnum=NULL;
        DUPNUM( pllrover->pnum, pll->pnum );
        addnum( &(pllrover->pnum), tmp, nRadix );
        pllrover->llprev = pll;
        pll = pllrover;
        }
    destroynum( tmp );
    cdigits = 0;
    while ( cdigits++ < thismax && !zernum(rem) )
        {
        pllrover = pll;
        digit = nRadix - 1;
        do    {
            bret = lessnum( rem, pllrover->pnum );
            } while ( bret && --digit && ( pllrover = pllrover->llprev ) );
        if ( digit )
            {
            pllrover->pnum->sign *= -1;
            addnum( &rem, pllrover->pnum, nRadix );
            pllrover->pnum->sign *= -1;
            }
        rem->exp++;
        *ptrc-- = (MANTTYPE)digit;
        }
    cdigits--;
    if ( MANT(c) != ++ptrc )
        {
        memcpy( MANT(c), ptrc, (int)(cdigits*sizeof(MANTTYPE)) );
        }
    
    // Cleanup table structure
    pllrover = pll;
    do    {
        pll = pllrover->llprev;
        destroynum( pllrover->pnum );
        zfree( pllrover );
        } while ( pllrover = pll );

    if ( !cdigits )
        {
        c->cdigit = 1;
        c->exp = 0;
        }
    else
        {
        c->cdigit = cdigits;
        c->exp -= cdigits;
        while ( c->cdigit > 1 && MANT(c)[c->cdigit-1] == 0 )
            {
            c->cdigit--;
            }
        }
    destroynum( rem );

    destroynum( *pa );
    *pa=c;
}


//---------------------------------------------------------------------------
//
//    FUNCTION: equnum
//
//    ARGUMENTS: two numbers.
//
//    RETURN: Boolean
//
//    DESCRIPTION: Does the number equivalent of ( a == b )
//    Only assumes that a and b are the same nRadix.
//
//---------------------------------------------------------------------------

BOOL equnum( PNUMBER a, PNUMBER b )

{
    long diff;
    MANTTYPE *pa;
    MANTTYPE *pb;
    long cdigits;
    long ccdigits;
    MANTTYPE  da;
    MANTTYPE  db;

    diff = ( a->cdigit + a->exp ) - ( b->cdigit + b->exp );
    if ( diff < 0 )
        {
        // If the exponents are different, these are different numbers.
        return( FALSE );
        }
    else
        {
        if ( diff > 0 )
            {
            // If the exponents are different, these are different numbers.
            return( FALSE );
            }
        else
            {
            // OK the exponents match.
            pa = MANT(a);
            pb = MANT(b);
            pa += a->cdigit - 1;
            pb += b->cdigit - 1;
            cdigits = max( a->cdigit, b->cdigit );
            ccdigits = cdigits;
            
            // Loop over all digits until we run out of digits or there is a
            // difference in the digits.
            for ( ;cdigits > 0; cdigits-- )
                {
                da = ( (cdigits > (ccdigits - a->cdigit) ) ? 
                    *pa-- : 0 );
                db = ( (cdigits > (ccdigits - b->cdigit) ) ? 
                    *pb-- : 0 );
                if ( da != db )
                    {
                    return( FALSE );
                    }
                }
            
            // In this case, they are equal.
            return( TRUE );
            }
        }
}

//---------------------------------------------------------------------------
//
//    FUNCTION: lessnum
//
//    ARGUMENTS: two numbers.
//
//    RETURN: Boolean
//
//    DESCRIPTION: Does the number equivalent of ( abs(a) < abs(b) )
//    Only assumes that a and b are the same nRadix, WARNING THIS IS AN.
//    UNSIGNED COMPARE!
//
//---------------------------------------------------------------------------

BOOL lessnum( PNUMBER a, PNUMBER b )

{
    long diff;
    MANTTYPE *pa;
    MANTTYPE *pb;
    long cdigits;
    long ccdigits;
    MANTTYPE  da;
    MANTTYPE  db;


    diff = ( a->cdigit + a->exp ) - ( b->cdigit + b->exp );
    if ( diff < 0 )
        {
        // The exponent of a is less than b
        return( TRUE );
        }
    else
        {
        if ( diff > 0 )
            {
            return( FALSE );
            }
        else
            {
            pa = MANT(a);
            pb = MANT(b);
            pa += a->cdigit - 1;
            pb += b->cdigit - 1;
            cdigits = max( a->cdigit, b->cdigit );
            ccdigits = cdigits;
            for ( ;cdigits > 0; cdigits-- )
                {
                da = ( (cdigits > (ccdigits - a->cdigit) ) ? 
                    *pa-- : 0 );
                db = ( (cdigits > (ccdigits - b->cdigit) ) ? 
                    *pb-- : 0 );
                diff = da-db;
                if ( diff )
                    {
                    return( diff < 0 );
                    }
                }
            // In this case, they are equal.
            return( FALSE );
            }
        }
}

//----------------------------------------------------------------------------
//
//    FUNCTION: zernum
//
//    ARGUMENTS: number
//
//    RETURN: Boolean
//
//    DESCRIPTION: Does the number equivalent of ( !a )
//
//----------------------------------------------------------------------------

BOOL zernum( PNUMBER a )

{
    long length;
    MANTTYPE *pcha;
    length = a->cdigit;
    pcha = MANT( a );
    
    // loop over all the digits until you find a nonzero or until you run 
    // out of digits
    while ( length-- > 0 )
        {
        if ( *pcha++ )
            {
            // One of the digits isn't zero, therefore the number isn't zero
            return( FALSE );
            }
        }
    // All of the digits are zero, therefore the number is zero
    return( TRUE );
}
