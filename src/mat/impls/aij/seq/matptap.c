
/*
  Defines projective product routines where A is a SeqAIJ matrix
          C = P^T * A * P
*/

#include <../src/mat/impls/aij/seq/aij.h>   /*I "petscmat.h" I*/
#include <../src/mat/utils/freespace.h>
#include <petscbt.h>

#undef __FUNCT__
#define __FUNCT__ "MatPtAPSymbolic_SeqAIJ"
PetscErrorCode MatPtAPSymbolic_SeqAIJ(Mat A,Mat P,PetscReal fill,Mat *C)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (!P->ops->ptapsymbolic_seqaij) SETERRQ2(((PetscObject)A)->comm,PETSC_ERR_SUP,"Not implemented for A=%s and P=%s",((PetscObject)A)->type_name,((PetscObject)P)->type_name);
  ierr = (*P->ops->ptapsymbolic_seqaij)(A,P,fill,C);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPtAPNumeric_SeqAIJ"
PetscErrorCode MatPtAPNumeric_SeqAIJ(Mat A,Mat P,Mat C)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  if (!P->ops->ptapnumeric_seqaij) SETERRQ2(((PetscObject)A)->comm,PETSC_ERR_SUP,"Not implemented for A=%s and P=%s",((PetscObject)A)->type_name,((PetscObject)P)->type_name);
  ierr = (*P->ops->ptapnumeric_seqaij)(A,P,C);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPtAPSymbolic_SeqAIJ_SeqAIJ"
PetscErrorCode MatPtAPSymbolic_SeqAIJ_SeqAIJ(Mat A,Mat P,PetscReal fill,Mat *C) 
{
  PetscErrorCode     ierr;
  PetscFreeSpaceList free_space=PETSC_NULL,current_space=PETSC_NULL;
  Mat_SeqAIJ         *a = (Mat_SeqAIJ*)A->data,*p = (Mat_SeqAIJ*)P->data,*c;
  PetscInt           *pti,*ptj,*ptJ,*ai=a->i,*aj=a->j,*ajj,*pi=p->i,*pj=p->j,*pjj;
  PetscInt           *ci,*cj,*ptadenserow,*ptasparserow,*ptaj,nspacedouble=0;
  PetscInt           an=A->cmap->N,am=A->rmap->N,pn=P->cmap->N;
  PetscInt           i,j,k,ptnzi,arow,anzj,ptanzi,prow,pnzj,cnzi,nlnk,*lnk;
  MatScalar          *ca;
  PetscBT            lnkbt;

  PetscFunctionBegin;
  /* Get ij structure of P^T */
  ierr = MatGetSymbolicTranspose_SeqAIJ(P,&pti,&ptj);CHKERRQ(ierr);
  ptJ=ptj;

  /* Allocate ci array, arrays for fill computation and */
  /* free space for accumulating nonzero column info */
  ierr = PetscMalloc((pn+1)*sizeof(PetscInt),&ci);CHKERRQ(ierr);
  ci[0] = 0;

  ierr = PetscMalloc((2*an+1)*sizeof(PetscInt),&ptadenserow);CHKERRQ(ierr);
  ierr = PetscMemzero(ptadenserow,(2*an+1)*sizeof(PetscInt));CHKERRQ(ierr);
  ptasparserow = ptadenserow  + an;

  /* create and initialize a linked list */
  nlnk = pn+1;
  ierr = PetscLLCreate(pn,pn,nlnk,lnk,lnkbt);CHKERRQ(ierr);

  /* Set initial free space to be fill*nnz(A). */
  /* This should be reasonable if sparsity of PtAP is similar to that of A. */
  ierr          = PetscFreeSpaceGet((PetscInt)(fill*ai[am]),&free_space);
  current_space = free_space;

  /* Determine symbolic info for each row of C: */
  for (i=0;i<pn;i++) {
    ptnzi  = pti[i+1] - pti[i];
    ptanzi = 0;
    /* Determine symbolic row of PtA: */
    for (j=0;j<ptnzi;j++) {
      arow = *ptJ++;
      anzj = ai[arow+1] - ai[arow];
      ajj  = aj + ai[arow];
      for (k=0;k<anzj;k++) {
        if (!ptadenserow[ajj[k]]) {
          ptadenserow[ajj[k]]    = -1;
          ptasparserow[ptanzi++] = ajj[k];
        }
      }
    }
    /* Using symbolic info for row of PtA, determine symbolic info for row of C: */
    ptaj = ptasparserow;
    cnzi   = 0;
    for (j=0;j<ptanzi;j++) {
      prow = *ptaj++;
      pnzj = pi[prow+1] - pi[prow];
      pjj  = pj + pi[prow];
      /* add non-zero cols of P into the sorted linked list lnk */
      ierr = PetscLLAdd(pnzj,pjj,pn,nlnk,lnk,lnkbt);CHKERRQ(ierr);
      cnzi += nlnk;
    }
   
    /* If free space is not available, make more free space */
    /* Double the amount of total space in the list */
    if (current_space->local_remaining<cnzi) {
      ierr = PetscFreeSpaceGet(cnzi+current_space->total_array_size,&current_space);CHKERRQ(ierr);
      nspacedouble++;
    }

    /* Copy data into free space, and zero out denserows */
    ierr = PetscLLClean(pn,pn,cnzi,lnk,current_space->array,lnkbt);CHKERRQ(ierr);
    current_space->array           += cnzi;
    current_space->local_used      += cnzi;
    current_space->local_remaining -= cnzi;
    
    for (j=0;j<ptanzi;j++) {
      ptadenserow[ptasparserow[j]] = 0;
    }
    /* Aside: Perhaps we should save the pta info for the numerical factorization. */
    /*        For now, we will recompute what is needed. */ 
    ci[i+1] = ci[i] + cnzi;
  }
  /* nnz is now stored in ci[ptm], column indices are in the list of free space */
  /* Allocate space for cj, initialize cj, and */
  /* destroy list of free space and other temporary array(s) */
  ierr = PetscMalloc((ci[pn]+1)*sizeof(PetscInt),&cj);CHKERRQ(ierr);
  ierr = PetscFreeSpaceContiguous(&free_space,cj);CHKERRQ(ierr);
  ierr = PetscFree(ptadenserow);CHKERRQ(ierr);
  ierr = PetscLLDestroy(lnk,lnkbt);CHKERRQ(ierr);
  
  /* Allocate space for ca */
  ierr = PetscMalloc((ci[pn]+1)*sizeof(MatScalar),&ca);CHKERRQ(ierr);
  ierr = PetscMemzero(ca,(ci[pn]+1)*sizeof(MatScalar));CHKERRQ(ierr);
  
  /* put together the new matrix */
  ierr = MatCreateSeqAIJWithArrays(((PetscObject)A)->comm,pn,pn,ci,cj,ca,C);CHKERRQ(ierr);

  /* MatCreateSeqAIJWithArrays flags matrix so PETSc doesn't free the user's arrays. */
  /* Since these are PETSc arrays, change flags to free them as necessary. */
  c = (Mat_SeqAIJ *)((*C)->data);
  c->free_a  = PETSC_TRUE;
  c->free_ij = PETSC_TRUE;
  c->nonew   = 0;

  /* Clean up. */
  ierr = MatRestoreSymbolicTranspose_SeqAIJ(P,&pti,&ptj);CHKERRQ(ierr);
#if defined(PETSC_USE_INFO)
  if (ci[pn] != 0) {
    PetscReal afill = ((PetscReal)ci[pn])/ai[am];
    if (afill < 1.0) afill = 1.0;
    ierr = PetscInfo3((*C),"Reallocs %D; Fill ratio: given %G needed %G.\n",nspacedouble,fill,afill);CHKERRQ(ierr);
    ierr = PetscInfo1((*C),"Use MatPtAP(A,P,MatReuse,%G,&C) for best performance.\n",afill);CHKERRQ(ierr);
  } else {
    ierr = PetscInfo((*C),"Empty matrix product\n");CHKERRQ(ierr);
  }
#endif
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatPtAPNumeric_SeqAIJ_SeqAIJ"
PetscErrorCode MatPtAPNumeric_SeqAIJ_SeqAIJ(Mat A,Mat P,Mat C) 
{
  PetscErrorCode ierr;
  Mat_SeqAIJ     *a  = (Mat_SeqAIJ *) A->data;
  Mat_SeqAIJ     *p  = (Mat_SeqAIJ *) P->data;
  Mat_SeqAIJ     *c  = (Mat_SeqAIJ *) C->data;
  PetscInt       *ai=a->i,*aj=a->j,*apj,*apjdense,*pi=p->i,*pj=p->j,*pcol;
  PetscInt       *ci=c->i,*cj=c->j,*cjj,cnz;
  PetscInt       am=A->rmap->N,cn=C->cmap->N,cm=C->rmap->N;
  PetscInt       i,j,k,anz,apnz,pnz,prow,crow,apcol,nextap;
  MatScalar      *aa=a->a,*apa,*pa=p->a,*pval,*ca=c->a,*caj;
  PetscBool      sparse_axpy=PETSC_FALSE;

  PetscFunctionBegin;
  /* Allocate temporary array for storage of one row of A*P */
  ierr = PetscMalloc3(cn,MatScalar,&apa,cn,PetscInt,&apj,cn,PetscInt,&apjdense);CHKERRQ(ierr);
  ierr = PetscMemzero(apa,cn*sizeof(MatScalar));CHKERRQ(ierr);
  ierr = PetscMemzero(apjdense,cn*sizeof(PetscInt));CHKERRQ(ierr);

  /* Clear old values in C */
  ierr = PetscMemzero(ca,ci[cm]*sizeof(MatScalar));CHKERRQ(ierr);

  for (i=0;i<am;i++) {
    /* Form sparse row of AP[i,:] = A[i,:]*P */
    anz  = ai[i+1] - ai[i];
    apnz = 0;
    for (j=0; j<anz; j++) {
      prow = aj[j];
      pnz  = pi[prow+1] - pi[prow];
      pcol = pj + pi[prow];
      pval = pa + pi[prow];
      for (k=0; k<pnz; k++) {
        if (!apjdense[pcol[k]]) {
          apjdense[pcol[k]] = -1; 
          apj[apnz++]       = pcol[k];
        }
        apa[pcol[k]] += aa[j]*pval[k];
      }
      ierr = PetscLogFlops(2.0*pnz);CHKERRQ(ierr);
    }
    aj += anz; aa += anz;

    if (sparse_axpy){
      ierr = PetscSortInt(apnz,apj);CHKERRQ(ierr);
    }

    /* Compute P^T*A*P using outer product P[i,:]^T*AP[i,:]. */
    pnz  = pi[i+1] - pi[i];
    pcol = pj + pi[i];
    pval = pa + pi[i];
    for (j=0; j<pnz; j++) {
      crow = pcol[j]; 
      cjj  = cj + ci[crow];
      caj  = ca + ci[crow];

      if (sparse_axpy){  /* Perform sparse axpy */
        nextap = 0;
        apcol = apj[nextap];
        for (k=0; nextap<apnz; k++) {
          if (cjj[k] == apcol) {
            caj[k] += pval[j]*apa[apcol];
            apcol   = apj[++nextap];
          }
        }
        ierr = PetscLogFlops(2.0*apnz);CHKERRQ(ierr);
      } else { /* Perform dense axpy */
        cnz  = ci[crow+1] - ci[crow];
        for (k=0; k<cnz; k++){
          caj[k] += pval[j]*apa[cjj[k]];
        }
        ierr = PetscLogFlops(2.0*cnz);CHKERRQ(ierr);
      }
    }

    /* Zero the current row info for A*P */
    for (j=0; j<apnz; j++) {
      apcol           = apj[j];
      apa[apcol]      = 0.;
      apjdense[apcol] = 0;
    }
  }

  /* Assemble the final matrix and clean up */
  ierr = MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = PetscFree3(apa,apj,apjdense);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
