#include "petscfunc.h"
#include "ramgfunc.h"
#include "petscsles.h"

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "KSPMonitorWriteConvHist"
/*
   KSPMonitorWriteConvHist - Write convergence history to external ASCII file. 

   Input Parameters:
     ksp   - iterative context
     n     - iteration number
     rnorm - 2-norm (preconditioned) residual value (may be estimated)
     dummy - optional user-defined monitor context (unused here)
*/
/*
  Sample usage: 
  ierr = KSPSetMonitor(ksp, KSPMonitorWriteConvHist,PETSC_NULL, 
                       PETSC_NULL); CHKERRA(ierr); 
   
  Note: the tolerance file can viewed using gnuplot, e.g. 
  gnuplot plotpetsctol 

*/
int KSPMonitorWriteConvHist(KSP ksp,int n,double rnorm,void* ctx)
{
  double   bnrm2;
  char     filename[161];
  FILE     *ftol;
  //  CONVHIST *convhist;

  //  convhist = (CONVHIST*)(ctx); 
  //  bnrm2 =    (*convhist).BNRM2;

  sprintf(filename,"petsctol"); 

  if (n == 0){
     PetscFOpen(MPI_COMM_WORLD,filename,"w",&ftol);
     //     PetscFPrintf(MPI_COMM_WORLD,ftol,"%14.12e \n",rnorm/bnrm2); 
     PetscFPrintf(MPI_COMM_WORLD,ftol,"%14.12e \n",rnorm); 
     PetscFClose(MPI_COMM_WORLD,ftol);
  }
  else if (n > 0) {
     PetscFOpen(MPI_COMM_WORLD,filename,"a",&ftol);
     //     PetscFPrintf(MPI_COMM_WORLD,ftol,"%14.12e \n",rnorm/bnrm2); 
     PetscFPrintf(MPI_COMM_WORLD,ftol,"%14.12e \n",rnorm); 
     PetscFClose(MPI_COMM_WORLD,ftol);
  }
  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "KSPMonitorAmg"
/*
   KSPMonitorWriteConvHist - Write convergence history to AMG-PETSc 
   interface external ASCII file. This routine differs from the previous one 
   by the fact that the index of each iteration is put in front of each 
   residual. 
    
   Input Parameters:
     ksp   - iterative context
     n     - iteration number
     rnorm - 2-norm (preconditioned) residual value (may be estimated)
     dummy - optional user-defined monitor context (unused here)
*/
int KSPMonitorAmg(KSP ksp,int n,double rnorm,void* ctx)
{
  double   bnrm2;
  char     filename[161];
  FILE     *ftol;
  //  CONVHIST *convhist;

  //  convhist = (CONVHIST*)(ctx); 
  //  bnrm2 =    (*convhist).BNRM2;

  sprintf(filename,"petsctol"); 

  if (n == 0){
     PetscFOpen(MPI_COMM_WORLD,filename,"w",&ftol);
     //     PetscFPrintf(MPI_COMM_WORLD,ftol,"%14.12e \n",rnorm/bnrm2); 
     PetscFPrintf(MPI_COMM_WORLD,ftol,"%d %14.12e \n",n, rnorm); 
     PetscFClose(MPI_COMM_WORLD,ftol);
  }
  else if (n > 0) {
     PetscFOpen(MPI_COMM_WORLD,filename,"a",&ftol);
     //     PetscFPrintf(MPI_COMM_WORLD,ftol,"%14.12e \n",rnorm/bnrm2); 
     PetscFPrintf(MPI_COMM_WORLD,ftol,"%d %14.12e \n",n, rnorm); 
     PetscFClose(MPI_COMM_WORLD,ftol);
  }
  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "KSPMonitorWriteResVecs"
/*
    KSPMonitorWriteResVecs - Write residual vectors to file. 
*/ 
int KSPMonitorWriteResVecs(KSP ksp,int n,double rnorm,void* ctx)
{
  Scalar   *values; 
  Vec      t, v, V; 
  Viewer   viewer; 
  char     filename[161];
  int      ierr, i, numnodes; 
  CONVHIST *convhist;
  FILE     *fout; 

  convhist = (CONVHIST*)(ctx); 
  numnodes = convhist->NUMNODES;

  sprintf(filename,"/home/domenico/Antas/Output/residual.%u",n); 
  ierr = VecCreate(MPI_COMM_WORLD,numnodes,numnodes,&t); CHKERRA(ierr);
  ierr = VecSetType(t,VEC_SEQ); CHKERRA(ierr);
  ierr = VecDuplicate(t,&v); CHKERRA(ierr); 

  ierr = KSPBuildResidual(ksp, t, v, &V); CHKERRA(ierr); 
  
  //  ierr = ViewerFileOpenASCII(MPI_COMM_WORLD,filename,&viewer); CHKERRA(ierr);
  //  ierr = ViewerSetFormat(viewer,VIEWER_FORMAT_ASCII_MATLAB,PETSC_NULL); 
  //         CHKERRA(ierr);
  //  ierr = VecView(V, viewer); CHKERRA(ierr);
  //  ierr = ViewerDestroy(viewer); CHKERRA(ierr);
  ierr = VecGetArray(V,&values); CHKERRA(ierr); 
  PetscFOpen(MPI_COMM_WORLD,filename,"w",&fout);
  for (i=0;i<numnodes;i++)
      PetscFPrintf(MPI_COMM_WORLD,fout,"%14.12e \n", values[i] ); 
  PetscFClose(MPI_COMM_WORLD,fout);

  ierr = VecRestoreArray(V,&values); CHKERRA(ierr);

  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "ConvhistCtxCreate"
int ConvhistCtxCreate(CONVHIST **convhist)
{
   CONVHIST *newctx = PetscNew(CONVHIST);  CHKPTRQ(newctx); 
   newctx->BNRM2 =0.;
   *convhist = newctx; 
   return 0; 
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "ConvhistDestroy"
int ConvhistCtxDestroy(CONVHIST *convhist)
{
   PetscFree(convhist);
   return 0; 
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "MyConvTest"
int MyConvTest(KSP ksp,int n, double rnorm, KSPConvergedReason *reason, 
               void* ctx)
{
  int ierr; 
  double   bnrm2, rtol; 
  CONVHIST *convhist = (CONVHIST*) ctx;
 
  bnrm2 =    convhist->BNRM2;
  ierr = KSPGetTolerances(ksp, &rtol, PETSC_NULL, PETSC_NULL, PETSC_NULL ); 
         CHKERRA(ierr);
  if (rnorm/bnrm2 < rtol){ 
    PetscPrintf(MPI_COMM_WORLD,"[test] %d %g \r",n,rnorm/bnrm2);
    return 1; }
  else{
    PetscPrintf(MPI_COMM_WORLD,"[test] %d %g \r",n,rnorm/bnrm2);
    return 0;}
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "ReorderSubmatrices"
int ReorderSubmatrices(PC pc,int nsub,IS *row,IS *col,Mat *submat,void *dummy)
{
  int               i, j, ierr, nloc, *glo_row_ind;
  IS                isrow,iscol;      /* row and column permutations */
  MatOrderingType   rtype = MATORDERING_RCM;

  for (i=0; i<nsub; i++) {
     ierr = MatGetOrdering(submat[i], rtype, &isrow, &iscol); CHKERRA(ierr);
  }

  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "PrintSubMatrices"
int PrintSubMatrices(PC pc,int nsub,IS *row,IS *col,Mat *submat,void *dummy)
{
  int    i, j, ierr, nloc, *glo_row_ind;

  PetscPrintf(PETSC_COMM_WORLD,"***  Overzicht van opdeling matrix *** \n");
  for (i=0; i<nsub; i++) {
    PetscPrintf(PETSC_COMM_WORLD,"\n** Jacobi Blok %d \n",i);
    ierr = ISGetSize(row[i],&nloc); CHKERRA(ierr); 
    ierr = ISGetIndices(row[i], &glo_row_ind); CHKERRA(ierr);
    for (j=0; j< nloc; j++) {
       PetscPrintf(PETSC_COMM_WORLD,"[%d] global row %d\n",j,glo_row_ind[j]); 
    }
  ierr = ISRestoreIndices(row[i],&glo_row_ind); CHKERRA(ierr);
  }

  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "ViewSubMatrices"
int ViewSubMatrices(PC pc,int nsub,IS *row,IS *col,Mat *submat,void *dummy)
{
  int    i, ierr;
  Viewer viewer; 
  Draw   draw; 

  for (i=0; i<nsub; i++) {
    // ierr = MatView(submat[i],PETSC_NULL); CHKERRA(ierr);
     ierr = ViewerDrawOpen(MPI_COMM_WORLD,PETSC_NULL, PETSC_NULL, 
            0, 0, 500,500,&viewer); 
     ierr = ViewerDrawGetDraw(viewer, 0, &draw); CHKERRA(ierr);
     ierr = MatView(submat[i], viewer); CHKERRA(ierr);
     ierr = DrawPause(draw); CHKERRA(ierr);
     ierr = ViewerDestroy(viewer); CHKERRA(ierr);
  }

  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "SamgShellPCSetUpOnFem"
int SamgShellPCSetUpOnFem(PC pc,int nsub,IS *row,IS *col,Mat *submat,void *ctx)
{
  int         ierr;
  int         nnn; 
  SamgShellPC *samg_ctx;

  samg_ctx = (SamgShellPC*) ctx; 
  ierr = MatGetSize(submat[0], &nnn, &nnn); CHKERRA(ierr);
  printf("\n Dim submatrix = %d\n\n",nnn); 
  ierr = SamgShellPCSetUp(samg_ctx,submat[0]); CHKERRA(ierr); 

  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__
#define __FUNC__ "MyMatView"
int MyMatView(Mat mat,void *dummy)
{
  int    i, ierr;
  Viewer viewer; 
  Draw   draw; 

  ierr = ViewerDrawOpen(MPI_COMM_WORLD,PETSC_NULL, PETSC_NULL, 
	 0, 0, 500,500,&viewer); 
  ierr = ViewerDrawGetDraw(viewer, 0, &draw); CHKERRA(ierr);
  ierr = MatView(mat, viewer); CHKERRA(ierr);
  ierr = DrawSetPause(draw, 5); CHKERRA(ierr);
  ierr = DrawPause(draw); CHKERRA(ierr);
  ierr = ViewerDestroy(viewer); CHKERRA(ierr);

  return 0;
}

/* ------------------------------------------------------------------- */
#undef __FUNC__  
#define __FUNC__ "PrintMatrix"
int PrintMatrix(Mat mat, char* path, char* base)
{
   int      ierr;
   Viewer   viewer; 
   char     filename[80]; 
   Scalar   *vals_getrow; 
   int      numrows, numcols, numnonzero, I, j, ncols_getrow, *cols_getrow;
   MatInfo  info;

   /*..Get size and number of unknowns of matrix..*/ 
   ierr = MatGetSize(mat, &numrows, &numcols); CHKERRQ(ierr);
   ierr = MatGetInfo(mat,MAT_LOCAL,&info); CHKERRQ(ierr); 
   numnonzero = (int)info.nz_used;

   /*..Set file and open file for reading..*/ 
   sprintf(filename, "%s%s", path, base);
   printf("   [PrintMatrix]::Generating file %s ...\n", filename); 
   ierr = ViewerASCIIOpen(MPI_COMM_WORLD,filename,&viewer); 
          CHKERRA(ierr);
 
   /*..Print file header..*/
   ierr = ViewerASCIIPrintf(viewer,"%% \n"); 
   if (numrows==numcols)    /*..square matrix..*/  
     ierr = ViewerASCIIPrintf(viewer,"%% %d %d \n", numrows, numnonzero); 
   else                     /*..rectangular matrix..*/ 
     ierr = ViewerASCIIPrintf(viewer,"%% %d %d %d \n", numrows, numcols,  
                                                    numnonzero); 
 
   /*..Print matrix to rowwise file..*/ 
   for (I=0;I<numrows;I++){
     /*....Get row I of matrix....*/
     ierr = MatGetRow(mat,I,&ncols_getrow,&cols_getrow,&vals_getrow); 
            CHKERRQ(ierr); 
     /*....Print row I of matrix....*/ 
     for (j=0;j<ncols_getrow;j++){
       #if defined(PETSC_USE_COMPLEX)
         ierr = ViewerASCIIPrintf( viewer,"%d %d %22.18e %22.18e\n", 
                I+1, cols_getrow[j]+1, real(vals_getrow[j]), 
                imag(vals_getrow[j]) ); 
       #else
         ierr = ViewerASCIIPrintf( viewer,"%d %d %22.18e \n", 
                I+1, cols_getrow[j]+1, vals_getrow[j] ); 
       #endif 
     }
   }

   /*..Close file..*/ 
   ierr = ViewerDestroy(viewer); CHKERRA(ierr);
   printf("   [PrintMatrix]::Done Generating file !\n", filename);       
   return 0; 
}

/* ------------------------------------------------------------------- */
#undef __FUNC__  
#define __FUNC__ "PrintVector"
int PrintVector(Vec vec, char* path, char* base) 
{
   int    ierr;
   Viewer viewer; 
   char   filename[80]; 
   Scalar *values; 
   int    size, i; 

   sprintf(filename, "%s%s%s", path, base, ".m");
   printf("   [PrintVector]::Generating file %s ...\n", filename); 
   ierr = VecGetSize(vec, &size); CHKERRA(ierr);
   values = (Scalar *) PetscMalloc(size * sizeof(Scalar)); 
            CHKPTRQ(values); 
   ierr = VecGetArray(vec, &values); CHKERRA(ierr);
   ierr = ViewerASCIIOpen(MPI_COMM_WORLD,filename,&viewer); 
          CHKERRA(ierr);
   ierr = ViewerASCIIPrintf(viewer,"function [z] = %s()\n",base); 
          CHKERRA(ierr);
   ierr = ViewerASCIIPrintf(viewer,"z = [\n"); 
          CHKERRA(ierr);
   for (i=0;i<size;i++){
     #if defined(PETSC_USE_COMPLEX)
       ierr = ViewerASCIIPrintf(viewer, "%22.18e %22.18e \n",
                                real( values[i] ), imag( values[i] )); 
              CHKERRA(ierr);
     #else 
       ierr = ViewerASCIIPrintf(viewer, "%22.18e \n", values[i] ); 
              CHKERRA(ierr);
     #endif 
   }
  ierr = ViewerASCIIPrintf(viewer,"]; \n"); 
          CHKERRA(ierr);
   ierr = ViewerDestroy(viewer); CHKERRA(ierr);
   ierr = VecRestoreArray(vec, &values); CHKERRA(ierr);
   printf("   [PrintVector]::Done Generating file !\n", filename);    
   return 0; 
}






