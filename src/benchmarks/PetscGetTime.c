/*$Id: PetscGetTime.c,v 1.12 2000/01/11 21:03:44 bsmith Exp bsmith $*/

#include "petsc.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  PLogDouble x,y;
  int        i,ierr;
  
  PetscInitialize(&argc,&argv,0,0);
 /* To take care of paging effects */
  ierr = PetscGetTime(&y);CHKERRA(ierr);

  for (i=0; i<2; i++) {
    ierr = PetscGetTime(&x);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);
    ierr = PetscGetTime(&y);CHKERRA(ierr);

    fprintf(stdout,"%-15s : %e sec\n","PetscGetTime",(y-x)/10.0);
  }

  PetscFinalize();
  PetscFunctionReturn(0);
}
