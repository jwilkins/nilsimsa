/*
 * Source: Digest-Nilsimsa-0.06; _nilsimsa.c, nilsimsa.h
 * Changes: 2005-04-14 Stephen Lewis <slewis@orcon.net.nz>
 * - stripped superfluous code
 * - some cleanups, reformatting
 * - refactored to provide more appropriate ruby interface
 *
 * NOTE - I haven't gotten around to fixing the previous comment
 * headers below this
 */

/*
 * chad's modifications for perl xs - Digest::Nilsimsa
 *
 * main() - removed (too many warnings)
 * accbuf - added, practically identical to accfile()
 * dprint() - added  (prints debug msgs to debug.txt)
 *
 * $Id: _nilsimsa.c,v 1.1 2002/05/20 22:29:07 chad Exp $
 */

/***************************************************************************
 * main.c  -  nilsimsa
 * -------------------
 * begin                : Fri Mar 16 01:41:08 EST 2001
 * copyright            : (C) 2001-2002 by cmeclax
 * email                : cmeclax@ixazon.dynip.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* NOTE - this should really use the support code from Digest, but
 * would require a ruby source tree to build in that case ( the
 * required headers don't seem to be generally installed :/ )
 */

#include <ruby.h>
#include <assert.h>

#define NSR_CODE_LEN 32
#define RB_NSR_IVAR "cdata" /* not prefixing @ makes innaccessible from ruby */

#define tran3(a,b,c,n) (((tran[((a)+(n))&255]^tran[(b)]*((n)+(n)+1))+tran[(c)^tran[n]])&255)


struct nsrecord {
  int acc[256];  /* counts each trigram's hash */
  char code[NSR_CODE_LEN]; /* the nilsimsa code as a bit vector */

  unsigned int chcount; /* number of characters processed so far */
  int lastch[4]; /* last 4 characters processed */
};

void nsr_init( struct nsrecord *a );
void nsr_digest( struct nsrecord *a );
void nsr_update(struct nsrecord *, char *buf,unsigned long len);
int nilsimsa(char *a,char *b);
/*void nsr_free( struct nsrecord *r );*/
static void filltran(void);
static void fillpopcount(void);

static struct nsrecord *get_nsr( VALUE obj );
VALUE rbns_init(int argc, VALUE *argv, VALUE self);
VALUE rbns_update(VALUE self, VALUE data);
VALUE rbns_nilsimsa(VALUE self, VALUE other);
VALUE rbns_digest(VALUE self);
void Init_nilsimsa_native(void);

unsigned char tran[256], popcount[256];

/* formerly clear() */
void
nsr_init( struct nsrecord *a ) {
  assert( a );

  memset(a->acc,0,sizeof(a->acc));
  memset(a->code,0,sizeof(a->code));

  a->chcount = 0;
  {
    int i;
    for (i=0; i<4; i++) {
      a->lastch[i] = -1;
    }
  }
}

static void
filltran(void) {
  int i,j,k;
  for (i=j=0;i<256;i++) {
    j=(j*53+1)&255;
    j+=j;
    if (j>255) {
      j-=255;
    }
    for (k=0;k<i;k++) {
      if (j==tran[k]) {
        j=(j+1)&255;
        k=0;
      }
    }
    tran[i]=j;
  }
}

static void
fillpopcount(void) {
  int i,j;
  memset(popcount,0,sizeof(popcount));

  for (i=0;i<256;i++) {
    for (j=0;j<8;j++) {
      popcount[i]+=1&(i>>j);
    }
  }
}


/* formerly accfile() */
void
nsr_update(struct nsrecord *a, char *buf, unsigned long len) {
  unsigned int idx;
  int *lastch=a->lastch; // convenience

  assert( a );

  for(idx=0; idx<len; idx++) {
    unsigned char ch = (unsigned char) buf[idx];
    a->chcount++;
    if (lastch[1]>=0)
      a->acc[tran3(ch,lastch[0],lastch[1],0)]++;
    if (lastch[2]>=0) {
      a->acc[tran3(ch,lastch[0],lastch[2],1)]++;
      a->acc[tran3(ch,lastch[1],lastch[2],2)]++;
    }
    if (lastch[3]>=0) {
      a->acc[tran3(ch,lastch[0],lastch[3],3)]++;
      a->acc[tran3(ch,lastch[1],lastch[3],4)]++;
      a->acc[tran3(ch,lastch[2],lastch[3],5)]++;
      a->acc[tran3(lastch[3],lastch[0],ch,6)]++;
      a->acc[tran3(lastch[3],lastch[2],ch,7)]++;
    }
    lastch[3]=lastch[2];
    lastch[2]=lastch[1];
    lastch[1]=lastch[0];
    lastch[0]=ch;
  }
}

/* formerly makecode() */
void
nsr_digest(struct nsrecord *a) {
  int i;
  int total=0;     /* total number of trigrams counted */
  int threshold=0; /* mean of all numbers in acc */

  assert( a );

  switch (a->chcount) {
    case 0:
    case 1:
    case 2:
      break;
    case 3:
      total = 1;
      break;
    case 4:
      total = 4;
      break;
    default:
      total = (8 * a->chcount) - 28;
      break;
  }

  threshold=total/256; /* round down because criterion is >threshold */

  memset(a->code,0,sizeof(a->code));
  for (i=0;i<256;i++) {
    a->code[i>>3]+=((a->acc[i]>threshold)<<(i&7));
  }
}

/* NOTE - assumes both of length 32 */
int
nilsimsa(char *a,char *b) {
  int i,bits=0;
  assert( a );
  assert( b );

  for (i=0;i<NSR_CODE_LEN;i++) {
    bits+=popcount[255&(a[i]^b[i])];
  }

  return 128-bits;
}

/*
   void
   nsr_free( struct nsrecord *r ) {
   if (r) {
   free( r );
   }
   }
   */


/*
 *
 * begin ruby wrapper functions 
 * 
 */

static struct nsrecord *
get_nsr( VALUE obj ) {
  VALUE wrapped;
  struct nsrecord *ret;

  if (!RTEST( rb_funcall( obj, rb_intern( "kind_of?" ), 1, 
          rb_eval_string("Nilsimsa")))) {
    /* FIXME should raise exception */
    return NULL;
  }

  wrapped = rb_iv_get( obj, RB_NSR_IVAR );
  if (Qnil == wrapped) {
    return NULL;
  }

  Data_Get_Struct( wrapped, struct nsrecord, ret );
  return ret;
}

VALUE
rbns_init(int argc, VALUE *argv, VALUE self) {
  VALUE wrapped_nsr;
  struct nsrecord *r;
  wrapped_nsr = Data_Make_Struct( rb_cObject, struct nsrecord,
      NULL, -1, r );
  rb_iv_set( self, RB_NSR_IVAR, wrapped_nsr );
  nsr_init( r );

  return rb_funcall2( self, rb_intern( "old_initialize" ), argc, argv );
}

VALUE
rbns_update(VALUE self, VALUE data) {
  struct nsrecord *r;
  char *chdata;
  long chdata_len;
  r = get_nsr( self );

  Check_Type( data, T_STRING );
  chdata = rb_str2cstr( data, &chdata_len );
  nsr_update( r, chdata, chdata_len );
  return data;
}

VALUE
rbns_nilsimsa(VALUE self, VALUE other) {
  long len;
  char *d1;
  char *d2;

  d1 = rb_str2cstr( rb_funcall( self, rb_intern( "digest" ), 0 ), &len );
  if (len < NSR_CODE_LEN) {
    return Qnil;
  }

  Check_Type( other, T_STRING );
  d2 = rb_str2cstr( other, &len );
  if (len < NSR_CODE_LEN) {
    return Qnil;
  }

  return INT2NUM( nilsimsa( d1, d2 ) );
}

VALUE
rbns_digest(VALUE self) {
  struct nsrecord *r=get_nsr( self );

  nsr_digest( r );

  /* reverse a newly created string of the digest */
  return rb_funcall( rb_str_new( r->code, NSR_CODE_LEN ), 
      rb_intern( "reverse"), 0 );
}

void
Init_nilsimsa_native(void) {
  VALUE rb_cNilsimsa;
  /* initialize invariant data */
  filltran();
  fillpopcount();

  /* this grafts itself over the top of an existing Nilsimsa class */
  rb_cNilsimsa = rb_eval_string( "Nilsimsa" );
  /* we'll call old_initialize from our new initialize */
  rb_define_alias( rb_cNilsimsa, "old_initialize", "initialize" );
  rb_define_method( rb_cNilsimsa, "initialize", rbns_init, -1 );
  rb_define_method( rb_cNilsimsa, "update", rbns_update, 1 );
  rb_define_method( rb_cNilsimsa, "nilsimsa", rbns_nilsimsa, 1 );
  rb_define_method( rb_cNilsimsa, "digest", rbns_digest, 0 );
}
