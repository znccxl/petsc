/* $Id: sseenabled.c,v 1.14 2001/07/20 20:25:12 buschelm Exp buschelm $ */
#include "petscsys.h" /*I "petscsys.h" I*/

#ifdef PETSC_HAVE_SSE

#include PETSC_HAVE_SSE
#define SSE_FEATURE_FLAG 0x2000000 /* Mask for bit 25 (from bit 0) */

#include <string.h>

#undef __FUNCT__
#define __FUNCT__ "PetscSSEHardwareTest"
int PetscSSEHardwareTest(PetscTruth *flag) {
  int  ierr;
  char *vendor;
  char Gen[13]="GenuineIntel";

  PetscFunctionBegin;
  ierr = PetscMalloc(13*sizeof(char),&vendor);CHKERRQ(ierr);
  strcpy(vendor,"************");
  CPUID_GET_VENDOR(vendor);
  if (!strcmp(vendor,Gen)) { 
    /* If GenuineIntel ... */
    unsigned long myeax,myebx,myecx,myedx;
    CPUID(CPUID_FEATURES,&myeax,&myebx,&myecx,&myedx);
    /* SSE Feature is indicated by Bit 25 of the EDX register */
    if (myedx & SSE_FEATURE_FLAG) {
      *flag = PETSC_TRUE;
    } else {
      *flag = PETSC_FALSE;
    }
  }
  ierr = PetscFree(vendor);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#ifdef PARCH_linux
#include <signal.h>
/* 
   Early versions of the Linux kernel disables SSE hardware because
   it does not know how to preserve the SSE state at a context switch.
   To detect this feature, try an sse instruction in another process.  
   If it works, great!  If not, an illegal instruction signal will be thrown,
   so catch it and return an error code. 
*/
#define PetscSSEOSEnabledTest(arg) PetscSSEOSEnabledTest_Linux(arg)

static void PetscSSEDisabledHandler(int sig) {
  signal(SIGILL,SIG_IGN);
  exit(-1);
}

#undef __FUNCT__
#define __FUNCT__ "PetscSSEOSEnabledTest_Linux"
int PetscSSEOSEnabledTest_Linux(PetscTruth *flag) {
  int status, pid = 0;
  PetscFunctionBegin;
  signal(SIGILL,PetscSSEDisabledHandler);
  pid = fork();
  if (pid==0) {
    SSE_SCOPE_BEGIN;
      XOR_PS(XMM0,XMM0);
    SSE_SCOPE_END;
    exit(0);
  } else {
    wait(&status);
  }
  if (!status) {
    *flag = PETSC_TRUE;
  } else {
    *flag = PETSC_FALSE;
  }
  PetscFunctionReturn(0);
}

#endif
#ifdef PARCH_win32
/* 
   Windows 95/98/NT4 should have a Windows Update/Service Patch which enables this hardware.
   Windows ME/2000 doesn't disable SSE Hardware 
*/
#define PetscSSEOSEnabledTest(arg) PetscSSEOSEnabledTest_TRUE(arg)

#undef __FUNCT__
#define __FUNCT__ "PetscSSEOSEnabledTest_TRUE"
int PetscSSEOSEnabledTest_TRUE(PetscTruth *flag) {
  PetscFunctionBegin;
  *flag = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#endif 
#else  /* Not defined PETSC_HAVE_SSE */

#define PetscSSEHardwareTest(arg) 0
#define PetscSSEOSEnabledTest(arg) 0

#endif /* defined PETSC_HAVE_SSE */

#undef __FUNCT__
#define __FUNCT__ "PetscSSEIsEnabled"
/*@C
     PetscSSEIsEnabled - Determines if Intel Streaming SIMD Extensions to the x86 instruction 
     set can be used.  Some operating systems do not allow the use of these instructions despite
     hardware availability.

     Collective on MPI_Comm

     Input Parameter:
.    comm - the MPI Communicator

     Output Parameters:
.    lflag - Local Flag:  PETSC_TRUE if enabled in this process
.    gflag - Global Flag: PETSC_TRUE if enabled for all processes in comm

     Level: developer
@*/
int PetscSSEIsEnabled(MPI_Comm comm,PetscTruth *lflag,PetscTruth *gflag) {
  int ierr;
  PetscFunctionBegin;
  *lflag = PETSC_FALSE;
  *gflag = PETSC_FALSE;
  ierr = PetscSSEHardwareTest(lflag);CHKERRQ(ierr);
  if (*lflag) {
    ierr = PetscSSEOSEnabledTest(lflag);CHKERRQ(ierr);
  }
  ierr = MPI_Allreduce(gflag,lflag,1,MPI_INT,MPI_LAND,comm);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}


