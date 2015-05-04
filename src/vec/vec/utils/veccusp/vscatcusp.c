#include <petsc/private/isimpl.h>
#include <petsc/private/vecimpl.h>             /*I "petscvec.h" I*/

PETSC_INTERN PetscErrorCode VecScatterCUSPIndicesCreate_PtoP(PetscInt,PetscInt*,PetscInt,PetscInt*,PetscCUSPIndices*);

#undef __FUNCT__
#define __FUNCT__ "VecScatterInitializeForGPU"
/*@
   VecScatterInitializeForGPU - Initializes a generalized scatter from one vector to
 another for GPU based computation.  Effectively, this function creates all the
 necessary indexing buffers and work vectors needed to move data only those data points
 in a vector which need to be communicated across ranks. This is done at the first time
 this function is called. Currently, this only used in the context of the parallel
 SpMV call in MatMult_MPIAIJCUSP (in mpi/mpicusp/mpiaijcusp.cu) or MatMult_MPIAIJCUSPARSE
 (in mpi/mpicusparse/mpiaijcusparse.cu). This function is executed before the call to
 MatMult. This enables the memory transfers to be overlapped with the MatMult SpMV kernel
 call.

   Input Parameters:
+  inctx - scatter context generated by VecScatterCreate()
.  x - the vector from which we scatter
-  mode - the scattering mode, usually SCATTER_FORWARD.  The available modes are:
    SCATTER_FORWARD or SCATTER_REVERSE

  Level: intermediate

.seealso: VecScatterCreate(), VecScatterEnd()
@*/
PetscErrorCode  VecScatterInitializeForGPU(VecScatter inctx,Vec x,ScatterMode mode)
{
  VecScatter_MPI_General *to,*from;
  PetscErrorCode         ierr;
  PetscInt               i,*indices,*sstartsSends,*sstartsRecvs,nrecvs,nsends,bs;
  PetscBool              isSeq1,isSeq2;

  PetscFunctionBegin;
  ierr = VecScatterIsSequential_Private((VecScatter_Common*)inctx->fromdata,&isSeq1);CHKERRQ(ierr);
  ierr = VecScatterIsSequential_Private((VecScatter_Common*)inctx->todata,&isSeq2);CHKERRQ(ierr);
  if (isSeq1 || isSeq2) {
    PetscFunctionReturn(0);
  }
  if (mode & SCATTER_REVERSE) {
    to     = (VecScatter_MPI_General*)inctx->fromdata;
    from   = (VecScatter_MPI_General*)inctx->todata;
  } else {
    to     = (VecScatter_MPI_General*)inctx->todata;
    from   = (VecScatter_MPI_General*)inctx->fromdata;
  }
  bs           = to->bs;
  nrecvs       = from->n;
  nsends       = to->n;
  indices      = to->indices;
  sstartsSends = to->starts;
  sstartsRecvs = from->starts;
  if (x->valid_GPU_array != PETSC_CUSP_UNALLOCATED && (nsends>0 || nrecvs>0)) {
    if (!inctx->spptr) {
      PetscInt k,*tindicesSends,*sindicesSends,*tindicesRecvs,*sindicesRecvs;
      PetscInt ns = sstartsSends[nsends],nr = sstartsRecvs[nrecvs];
      /* Here we create indices for both the senders and receivers. */
      ierr = PetscMalloc1(ns,&tindicesSends);CHKERRQ(ierr);
      ierr = PetscMalloc1(nr,&tindicesRecvs);CHKERRQ(ierr);

      ierr = PetscMemcpy(tindicesSends,indices,ns*sizeof(PetscInt));CHKERRQ(ierr);
      ierr = PetscMemcpy(tindicesRecvs,from->indices,nr*sizeof(PetscInt));CHKERRQ(ierr);

      ierr = PetscSortRemoveDupsInt(&ns,tindicesSends);CHKERRQ(ierr);
      ierr = PetscSortRemoveDupsInt(&nr,tindicesRecvs);CHKERRQ(ierr);

      ierr = PetscMalloc1(bs*ns,&sindicesSends);CHKERRQ(ierr);
      ierr = PetscMalloc1(from->bs*nr,&sindicesRecvs);CHKERRQ(ierr);

      /* sender indices */
      for (i=0; i<ns; i++) {
        for (k=0; k<bs; k++) sindicesSends[i*bs+k] = tindicesSends[i]+k;
      }
      ierr = PetscFree(tindicesSends);CHKERRQ(ierr);

      /* receiver indices */
      for (i=0; i<nr; i++) {
        for (k=0; k<from->bs; k++) sindicesRecvs[i*from->bs+k] = tindicesRecvs[i]+k;
      }
      ierr = PetscFree(tindicesRecvs);CHKERRQ(ierr);

      /* create GPU indices, work vectors, ... */
      ierr = VecScatterCUSPIndicesCreate_PtoP(ns*bs,sindicesSends,nr*from->bs,sindicesRecvs,(PetscCUSPIndices*)&inctx->spptr);CHKERRQ(ierr);
      ierr = PetscFree(sindicesSends);CHKERRQ(ierr);
      ierr = PetscFree(sindicesRecvs);CHKERRQ(ierr);
    }
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "VecScatterFinalizeForGPU"
/*@
   VecScatterFinalizeForGPU - Finalizes a generalized scatter from one vector to
 another for GPU based computation. Effectively, this function resets the temporary
 buffer flags. Currently, this only used in the context of the parallel SpMV call in
 in MatMult_MPIAIJCUSP (in mpi/mpicusp/mpiaijcusp.cu) or MatMult_MPIAIJCUSPARSE
 (in mpi/mpicusparse/mpiaijcusparse.cu). Once the MatMultAdd is finished,
 the GPU temporary buffers used for messaging are no longer valid.

   Input Parameters:
+  inctx - scatter context generated by VecScatterCreate()

  Level: intermediate

@*/
PetscErrorCode  VecScatterFinalizeForGPU(VecScatter inctx)
{
  PetscFunctionBegin;
  PetscFunctionReturn(0);
}

