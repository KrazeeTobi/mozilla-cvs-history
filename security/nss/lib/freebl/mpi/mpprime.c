/*
 *  mpprime.c
 *
 *  Utilities for finding and working with prime and pseudo-prime
 *  integers
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the MPI Arbitrary Precision Integer Arithmetic
 * library.
 *
 * The Initial Developer of the Original Code is Michael J. Fromberger.
 * Portions created by Michael J. Fromberger are 
 * Copyright (C) 1997, 1998, 1999, 2000 Michael J. Fromberger. 
 * All Rights Reserved.
 *
 * Contributor(s):
 *	Netscape Communications Corporation
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable
 * instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the GPL.
 */

#include "mpprime.h"
#include "mplogic.h"
#include <stdlib.h>
#include <string.h>

#define SMALL_TABLE 0 /* determines size of hard-wired prime table */

#define RANDOM() rand()

#include "primes.c"  /* pull in the prime digit table */

/* 
   Test if any of a given vector of digits divides a.  If not, MP_NO
   is returned; otherwise, MP_YES is returned and 'which' is set to
   the index of the integer in the vector which divided a.
 */
mp_err    s_mpp_divp(mp_int *a, const mp_digit *vec, int size, int *which);

extern mp_err s_mp_pad(mp_int *mp, mp_size min); /* left pad with zeroes  */

/* {{{ mpp_divis(a, b) */

/*
  mpp_divis(a, b)

  Returns MP_YES if a is divisible by b, or MP_NO if it is not.
 */

mp_err  mpp_divis(mp_int *a, mp_int *b)
{
  mp_err  res;
  mp_int  rem;

  if((res = mp_init(&rem)) != MP_OKAY)
    return res;

  if((res = mp_mod(a, b, &rem)) != MP_OKAY)
    goto CLEANUP;

  if(mp_cmp_z(&rem) == 0)
    res = MP_YES;
  else
    res = MP_NO;

CLEANUP:
  mp_clear(&rem);
  return res;

} /* end mpp_divis() */

/* }}} */

/* {{{ mpp_divis_d(a, d) */

/*
  mpp_divis_d(a, d)

  Return MP_YES if a is divisible by d, or MP_NO if it is not.
 */

mp_err  mpp_divis_d(mp_int *a, mp_digit d)
{
  mp_err     res;
  mp_digit   rem;

  ARGCHK(a != NULL, MP_BADARG);

  if(d == 0)
    return MP_NO;

  if((res = mp_mod_d(a, d, &rem)) != MP_OKAY)
    return res;

  if(rem == 0)
    return MP_YES;
  else
    return MP_NO;

} /* end mpp_divis_d() */

/* }}} */

/* {{{ mpp_random(a) */

/*
  mpp_random(a)

  Assigns a random value to a.  This value is generated using the
  standard C library's rand() function, so it should not be used for
  cryptographic purposes, but it should be fine for primality testing,
  since all we really care about there is good statistical properties.

  As many digits as a currently has are filled with random digits.
 */

mp_err  mpp_random(mp_int *a)

{
  mp_digit  next = 0;
  int       ix, jx;

  ARGCHK(a != NULL, MP_BADARG);

  for(ix = 0; ix < USED(a); ix++) {
    for(jx = 0; jx < sizeof(mp_digit); jx++) {
      next = (next << CHAR_BIT) | (RANDOM() & UCHAR_MAX);
    }
    DIGIT(a, ix) = next;
  }

  return MP_OKAY;

} /* end mpp_random() */

/* }}} */

/* {{{ mpp_random_size(a, prec) */

mp_err  mpp_random_size(mp_int *a, mp_size prec)
{
  mp_err   res;

  ARGCHK(a != NULL && prec > 0, MP_BADARG);
  
  if((res = s_mp_pad(a, prec)) != MP_OKAY)
    return res;

  return mpp_random(a);

} /* end mpp_random_size() */

/* }}} */

/* {{{ mpp_divis_vector(a, vec, size, which) */

/*
  mpp_divis_vector(a, vec, size, which)

  Determines if a is divisible by any of the 'size' digits in vec.
  Returns MP_YES and sets 'which' to the index of the offending digit,
  if it is; returns MP_NO if it is not.
 */

mp_err  mpp_divis_vector(mp_int *a, const mp_digit *vec, int size, int *which)
{
  ARGCHK(a != NULL && vec != NULL && size > 0, MP_BADARG);
  
  return s_mpp_divp(a, vec, size, which);

} /* end mpp_divis_vector() */

/* }}} */

/* {{{ mpp_divis_primes(a, np) */

/*
  mpp_divis_primes(a, np)

  Test whether a is divisible by any of the first 'np' primes.  If it
  is, returns MP_YES and sets *np to the value of the digit that did
  it.  If not, returns MP_NO.
 */
mp_err  mpp_divis_primes(mp_int *a, mp_digit *np)
{
  int     size, which;
  mp_err  res;

  ARGCHK(a != NULL && np != NULL, MP_BADARG);

  size = (int)*np;
  if(size > prime_tab_size)
    size = prime_tab_size;

  res = mpp_divis_vector(a, prime_tab, size, &which);
  if(res == MP_YES) 
    *np = prime_tab[which];

  return res;

} /* end mpp_divis_primes() */

/* }}} */

/* {{{ mpp_fermat(a, w) */

/*
  Using w as a witness, try pseudo-primality testing based on Fermat's
  little theorem.  If a is prime, and (w, a) = 1, then w^a == w (mod
  a).  So, we compute z = w^a (mod a) and compare z to w; if they are
  equal, the test passes and we return MP_YES.  Otherwise, we return
  MP_NO.
 */
mp_err  mpp_fermat(mp_int *a, mp_digit w)
{
  mp_int  base, test;
  mp_err  res;
  
  if((res = mp_init(&base)) != MP_OKAY)
    return res;

  mp_set(&base, w);

  if((res = mp_init(&test)) != MP_OKAY)
    goto TEST;

  /* Compute test = base^a (mod a) */
  if((res = mp_exptmod(&base, a, a, &test)) != MP_OKAY)
    goto CLEANUP;

  
  if(mp_cmp(&base, &test) == 0)
    res = MP_YES;
  else
    res = MP_NO;

 CLEANUP:
  mp_clear(&test);
 TEST:
  mp_clear(&base);

  return res;

} /* end mpp_fermat() */

/* }}} */

/*
  Perform the fermat test on each of the primes in a list until
  a) one of them shows a is not prime, or 
  b) the list is exhausted.
  Returns:  MP_YES if it passes tests.
	    MP_NO  if fermat test reveals it is composite
	    Some MP error code if some other error occurs.
 */
mp_err mpp_fermat_list(mp_int *a, const mp_digit *primes, unsigned int nPrimes)
{
  mp_err rv = MP_YES;

  while (nPrimes-- > 0 && rv == MP_YES) {
    rv = mpp_fermat(a, *primes++);
  }
  return rv;
}

/* {{{ mpp_pprime(a, nt) */

/*
  mpp_pprime(a, nt)

  Performs nt iteration of the Miller-Rabin probabilistic primality
  test on a.  Returns MP_YES if the tests pass, MP_NO if one fails.
  If MP_NO is returned, the number is definitely composite.  If MP_YES
  is returned, it is probably prime (but that is not guaranteed).
 */

mp_err  mpp_pprime(mp_int *a, int nt)
{
  mp_err   res;
  mp_int   x, amo, m, z;	/* "amo" = "a minus one" */
  int      iter, jx, b;

  ARGCHK(a != NULL, MP_BADARG);

  /* Initialize temporaries... */
  if((res = mp_init(&amo)) != MP_OKAY)
    return res;
  /* Compute amo = a - 1 for what follows...    */
  if ((res = mp_sub_d(a, 1, &amo)) != MP_OKAY)
    goto X;

  /* How many times does 2 divide (a - 1)?    */
  for (b = 0; (res = mpl_get_bit(&amo, b)) == 0; ++b) {
    /* do nothing */
  }
  if (res < 0)
    goto X;
  if (!b) { /* a was even ? */
    res = MP_NO;
    goto X;
  }

  if((res = mp_init_size(&x, USED(a))) != MP_OKAY)
    goto X;
  if((res = mp_init(&z)) != MP_OKAY)
    goto Z;
  if ((res = mp_init(&m)) != MP_OKAY)
    goto M;
  if ((res = mp_div_2d(&amo, b, &m, 0)) != MP_OKAY)
    goto CLEANUP;

  /* Do the test nt times... */
  for(iter = 0; iter < nt; iter++) {

    /* Choose a random value for x < a          */
    s_mp_pad(&x, USED(a));
    mpp_random(&x);
    if((res = mp_mod(&x, a, &x)) != MP_OKAY)
      goto CLEANUP;

    /* Compute z = (x ** m) mod a               */
    if((res = mp_exptmod(&x, &m, a, &z)) != MP_OKAY)
      goto CLEANUP;
    
    if(mp_cmp_d(&z, 1) == 0 || mp_cmp(&z, &amo) == 0) {
      res = MP_YES;
      continue;
    }
    
    for (jx = 1; jx < b; jx++) {
      /* z = z^2 (mod a) */
      if((res = mp_sqrmod(&z, a, &z)) != MP_OKAY)
	goto CLEANUP;
	
      if(mp_cmp_d(&z, 1) == 0) {
	res = MP_NO;
	break;
      }
      if(mp_cmp(&z, &amo) == 0) {
	res = MP_YES;
	break;
      } 
      res = MP_NO;
    } /* end testing loop */

    /* If the test passes, we will continue iterating, but a failed
       test means the candidate is definitely NOT prime, so we will
       immediately break out of this loop
     */
    if(res == MP_NO)
      break;

  } /* end iterations loop */
  
CLEANUP:
  mp_clear(&m);
M:
  mp_clear(&z);
Z:
  mp_clear(&x);
X:
  mp_clear(&amo);
  return res;

} /* end mpp_pprime() */

/* }}} */

/* Produce table of composites from list of primes and trial value.  
** trial must be odd. List of primes must not include 2.
** sieve should have dimension >= MAXPRIME/2, where MAXPRIME is largest 
** prime in list of primes.  After this function is finished,
** if sieve[i] is non-zero, then (trial + 2*i) is composite.
** Each prime used in the sieve costs one division of trial, and eliminates
** one or more values from the search space. (3 eliminates 1/3 of the values
** alone!)  Each value left in the search space costs 1 or more modular 
** exponentations.  So, these divisions are a bargain!
*/
mp_err mpp_sieve(mp_int *trial, const mp_digit *primes, unsigned int nPrimes, 
		 unsigned char *sieve, unsigned int nSieve)
{
  mp_err       res;
  mp_digit     rem;
  unsigned int ix;
  unsigned long offset;

  memset(sieve, 0, nSieve);

  for(ix = 0; ix < nPrimes; ix++) {
    mp_digit prime = primes[ix];
    unsigned int i;
    if((res = mp_mod_d(trial, prime, &rem)) != MP_OKAY) 
      return res;

    if (rem == 0) {
      offset = 0;
    } else {
      offset = prime - (rem / 2);
    }
    for (i = offset; i < nSieve ; i += prime) {
      sieve[i] = 1;
    }
  }

  return MP_OKAY;
}

mp_err mpp_make_prime(mp_int *start, unsigned int nBits, unsigned int strong,
		      unsigned long * nTries)
{
  mp_digit      np;
  mp_err        res;
  int           i;
  mp_int        trial;
  mp_int        q;
  unsigned int  num_tests;
  unsigned char sieve[32*1024];

  ARGCHK(start != 0, MP_BADARG);
  ARGCHK(nBits > 16, MP_RANGE);

  mp_init(&trial);
  mp_init(&q);
  if (nBits >= 1024) {
    num_tests = 5;
  } else if (nBits >= 512) {
    num_tests = 7;
  } else if (nBits >= 384) {
    num_tests = 9;
  } else if (nBits >= 256) {
    num_tests = 13;
  }

  if (strong) 
    --nBits;
  res = mpl_set_bit(start, nBits - 1, 1);  if (res != MP_OKAY) goto loser;
  res = mpl_set_bit(start,         0, 1);  if (res != MP_OKAY) goto loser;
  for (i = mpl_significant_bits(start) - 1; i >= nBits; --i) {
    res = mpl_set_bit(start, i, 0);  if (res != MP_OKAY) goto loser;
  }
  /* start sieveing with prime value of 3. */
  res = mpp_sieve(start, prime_tab + 1, prime_tab_size - 1, 
			 sieve, sizeof sieve);
  if (res != MP_OKAY) goto loser;
#ifdef DEBUG
  res = 0;
  for (i = 0; i < sizeof sieve; ++i) {
    if (!sieve[i])
      ++res;
  }
  fprintf(stderr,"sieve found %d potential primes.\n", res);
#define FPUTC(x,y) fputc(x,y)
#else
#define FPUTC(x,y) 
#endif
  res = MP_NO;
  for(i = 0; i < sizeof sieve; ++i) {
    if (sieve[i])	/* this number is composite */
      continue;
    res = mp_add_d(start, 2 * i, &trial); if (res != MP_OKAY) goto loser;
    FPUTC('.', stderr);
    /* run a Fermat test */
    res = mpp_fermat(&trial, 2);
    if (res != MP_OKAY) {
      if (res == MP_NO)
	continue;	/* was composite */
      goto loser;
    }
      
    FPUTC('+', stderr);
    /* If that passed, run some Miller-Rabin tests	*/
    res = mpp_pprime(&trial, num_tests);
    if (res != MP_OKAY) {
      if (res == MP_NO)
	continue;	/* was composite */
      goto loser;
    }
    FPUTC('!', stderr);

    if (!strong) 
      break;	/* success !! */

    /* At this point, we have strong evidence that our candidate
       is itself prime.  If we want a strong prime, we need now
       to test q = 2p + 1 for primality...
     */
    mp_mul_2(&trial, &q);
    mp_add_d(&q, 1, &q);

    /* Test q for small prime divisors ... */
    np = prime_tab_size;
    if (mpp_divis_primes(&q, &np) == MP_YES) { /* is composite */
      mp_clear(&q);
      continue;
    }

    /* And test with Fermat, as with its parent ... */
    res = mpp_fermat(&q, 2);
    if (res != MP_YES) {
      mp_clear(&q);
      if (res == MP_NO)
	continue;	/* was composite */
      goto loser;
    }

    /* And test with Miller-Rabin, as with its parent ... */
    res = mpp_pprime(&q, num_tests);
    if (res != MP_YES) {
      mp_clear(&q);
      if (res == MP_NO)
	continue;	/* was composite */
      goto loser;
    }

    /* If it passed, we've got a winner */
    mp_exch(&q, &trial);
    mp_clear(&q);
    break;

  } /* end of loop through sieved values */
  if (res == MP_YES)
    mp_exch(&trial, start);
loser:
  mp_clear(&trial);
  if (nTries)
    *nTries += i;
  return res;
}

/*========================================================================*/
/*------------------------------------------------------------------------*/
/* Static functions visible only to the library internally                */

/* {{{ s_mpp_divp(a, vec, size, which) */

/* 
   Test for divisibility by members of a vector of digits.  Returns
   MP_NO if a is not divisible by any of them; returns MP_YES and sets
   'which' to the index of the offender, if it is.  Will stop on the
   first digit against which a is divisible.
 */

mp_err    s_mpp_divp(mp_int *a, const mp_digit *vec, int size, int *which)
{
  mp_err    res;
  mp_digit  rem;

  int     ix;

  for(ix = 0; ix < size; ix++) {
    if((res = mp_mod_d(a, vec[ix], &rem)) != MP_OKAY) 
      return res;

    if(rem == 0) {
      if(which)
	*which = ix;
      return MP_YES;
    }
  }

  return MP_NO;

} /* end s_mpp_divp() */

/* }}} */

/*------------------------------------------------------------------------*/
/* HERE THERE BE DRAGONS                                                  */
