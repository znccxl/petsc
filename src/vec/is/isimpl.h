/* $Id: isimpl.h,v 1.24 2000/09/22 20:42:51 bsmith Exp bsmith $ */

/*
    Index sets for scatter-gather type operations in vectors
and matrices. 

*/

#if !defined(_IS_H)
#define _IS_H
#include "petscis.h"

struct _ISOps {
  int  (*getsize)(IS,int*),
       (*getlocalsize)(IS,int*),
       (*getindices)(IS,int**),
       (*restoreindices)(IS,int**),
       (*invertpermutation)(IS,int,IS*),
       (*sortindices)(IS),
       (*sorted)(IS,PetscTruth *),
       (*duplicate)(IS,IS *),
       (*destroy)(IS),
       (*view)(IS,Viewer),
       (*identity)(IS,PetscTruth*);
};

struct _p_IS {
  PETSCHEADER(struct _ISOps)
  PetscTruth   isperm;          /* if is a permutation */
  int          max,min;         /* range of possible values */
  void         *data;
  PetscTruth   isidentity;
};


#endif
