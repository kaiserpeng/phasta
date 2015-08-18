/* This file provides interface functions for 'partial ' random
   access into the PHASTA input files

   Anil Karanam March 2001 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "mpi.h"
#include "phastaIO.h"
#include "rdtsc.h"
#include <FCMangle.h>
#include "new_interface.h"
#include "phIO.h"
#include "syncio.h"
#include "posixio.h"
#include "common_c.h"

#ifdef intel
#include <winsock2.h>
#else
#include <unistd.h>
#include <strings.h>
#endif

void igetMinMaxAvg(int *ivalue, double *stats, int *statRanks) {
  int isThisRank;
  double *value = (double*)malloc(sizeof(double));
  *value = 1.0*(*ivalue);
  rgetMinMaxAvg(value,stats,statRanks);
  free(value);
}

void rgetMinMaxAvg(double *value, double *stats, int *statRanks) {
  int isThisRank;

  MPI_Allreduce(value,&stats[0],1,MPI_DOUBLE,MPI_MIN,MPI_COMM_WORLD);
  isThisRank=workfc.numpe+1;
  if(*value==stats[0])
    isThisRank=workfc.myrank; 
  MPI_Allreduce(&isThisRank,&statRanks[0],1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);

  MPI_Allreduce(value,&stats[1],1,MPI_DOUBLE,MPI_MAX,MPI_COMM_WORLD);
  isThisRank=workfc.numpe+1;
  if(*value==stats[1])
    isThisRank=workfc.myrank; 
  MPI_Allreduce(&isThisRank,&statRanks[1],1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);

  MPI_Allreduce(value,&stats[2],1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
  stats[2] /= workfc.numpe;

  double sqValue = (*value)*(*value), sqValueAvg = 0.;
  MPI_Allreduce(&sqValue,&sqValueAvg,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
  sqValueAvg /= workfc.numpe;

  stats[3] = sqrt(sqValueAvg-stats[2]*stats[2]);
}

void print_mesh_stats(void) {
  int statRanks[2];
  double iStats[4], rStats[4];

  igetMinMaxAvg(&conpar.nshg,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("nshg    : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&conpar.numel,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("numel   : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&conpar.numelb,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("numelb  : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&conpar.nnz_tot,iStats,statRanks);
  if(workfc.myrank==workfc.master) {
    printf("nnz_tot : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
    printf("\n");
  }
}

void print_mpi_stats(void) {
  int statRanks[2];
  double iStats[4], rStats[4];

// NS equations
  igetMinMaxAvg(&mpistats.iISend,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iISend : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&mpistats.iIRecv,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iIRecv : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&mpistats.iWaitAll,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iWtAll : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&mpistats.iAllR,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iAllR  : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);

  rgetMinMaxAvg(&mpistats.rISend,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rISend : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rIRecv,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rIRecv : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rWaitAll,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rWtAll : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rCommu,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rCommu : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rAllR,rStats,statRanks);
  if(workfc.myrank==workfc.master) {
    printf("rAllR  : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
    printf("\n");
  }
// Scalars
  igetMinMaxAvg(&mpistats.iISendScal,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iISendScal : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&mpistats.iIRecvScal,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iIRecvScal : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&mpistats.iWaitAllScal,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iWtAllScal : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);
  igetMinMaxAvg(&mpistats.iAllRScal,iStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("iAllRScal : min [%d,%d], max[%d,%d] and avg[.,%d] (rms=%d)\n",statRanks[0],(int)iStats[0],statRanks[1],(int)iStats[1],(int)iStats[2],(int)iStats[3]);

  rgetMinMaxAvg(&mpistats.rISendScal,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rISendScal : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rIRecvScal,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rIRecvScal : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rWaitAllScal,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rWtAllScal : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rCommuScal,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rCommuScal : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
  rgetMinMaxAvg(&mpistats.rAllRScal,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("rAllRScal  : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);


}

void print_system_stats(double *tcorecp, double *tcorecpscal) {
  int statRanks[2];
  double iStats[4], rStats[4];
  double syst_assembly, syst_solve;

// NS equations
  syst_assembly = tcorecp[0];
  syst_solve = tcorecp[1];

  rgetMinMaxAvg(&syst_assembly,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("Elm. form. : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);

  rgetMinMaxAvg(&syst_solve,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("Lin. alg. sol : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);

// Scalars
  syst_assembly = tcorecpscal[0];
  syst_solve = tcorecpscal[1];

  rgetMinMaxAvg(&syst_assembly,rStats,statRanks);
  if(workfc.myrank==workfc.master)
    printf("Elm. form. Scal. : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);

  rgetMinMaxAvg(&syst_solve,rStats,statRanks);
  if(workfc.myrank==workfc.master) {
    printf("Lin. alg. sol Scal. : min [%d,%2.5f], max[%d,%2.5f] and avg[.,%2.5f] (rms=%2.5f)\n",statRanks[0],rStats[0],statRanks[1],rStats[1],rStats[2],rStats[3]);
    printf("\n");
  }
}



void countfieldstowriterestart()
{
  int nfields = 2; //solution, time derivatives

  if(outpar.ivort == 1){
    nfields++; //vorticity
  }

  if(abs(turbvar.itwmod) != 1 && outpar.iowflux == 1) { 
    nfields++; //instantaneous wss in bflux.f
  }

  if(timdat.istep == inpdat.nstep[timdat.itseq-1]){ //Last time step of the computation

    //projection vectors and pressure projection vectors (call saveLesRestart in itrdrv)
    if(matdat.matflg[0][0] ==-1) {
        nfields = nfields +2;
    }

    //if Print Error Indicators = true (call write_error in itrdrv)
    if(turbvar.ierrcalc == 1){
      nfields++;
    }

    //if Print ybar = True (call write_field(myrank,'a','ybar',4,... in itrdrv)
    if(outpar.ioybar == 1){
      nfields++;  //ybar

      //phase average fields
      if(outpar.nphasesincycle >0) {
        nfields = nfields + outpar.nphasesincycle;
      }

      if(abs(turbvar.itwmod) != 1 && outpar.iowflux == 1) { 
        nfields++; //wssbar
      }

    }

    if(turbvari.irans < 0) {
      nfields++; //dwal
    }

  }

  outpar.nsynciofieldswriterestart = nfields;

  if(workfc.myrank == 0) {
    printf("Number of fields to write in restart files: %d\n", nfields);
  }
}


void
Write_Restart(  int* pid,
                int* stepno,
                int* nshg,
                int* numVars,
                double* array1,
                double* array2 ) {

    char fname[255];
    char rfile[60];
    char existingfile[30], linkfile[30];
    int irstou;
    int magic_number = 362436;
    int* mptr = &magic_number;
    double version=0.0;
    int isize, nitems;
    int iarray[10];
    int nfiles;
    int nfields;
    int numparts;
    int irank;
    int nprocs;
    int nppf;

    //  First, count the number of fields to write and store the result in
    countfieldstowriterestart();

    //  Retrieve and compute the parameters required for SyncIO
    nfiles = outpar.nsynciofiles;
    nfields = outpar.nsynciofieldswriterestart;
    numparts = workfc.numpe;
    irank = *pid; //workfc.myrank;
    nprocs = workfc.numpe;
    int nppp = numparts/nprocs;   // always 1 for PHASTA
    int descriptor;
    char filename[255];
    bzero((void*)filename,255);

    if(nfiles == -1 ){
      streamio_setup(rstream, &f_descriptor, 'w');
    } if(nfiles == 0 ){
      sprintf(filename,"restart.%d.", *stepno);
      posixio_setup(&f_descriptor, 'w');
    } else {
      nppf=numparts/nfiles;
      sprintf(filename,"restart-dat.%d.", *stepno);
      syncio_setup_write(nfiles, nfields, nppf, &f_descriptor);
    }
    phio_openfile(filename, f_descriptor);

    field_flag=0;


     int i;
     for ( i = 0; i < nppp; i++) { //This loop is useful only if several parts per processor
        // Write solution field ...
        isize = (*nshg)*(*numVars);
        nitems = 3;
        iarray[ 0 ] = (*nshg);
        iarray[ 1 ] = (*numVars);
        iarray[ 2 ] = (*stepno);

        phio_writeheader(f_descriptor, "solution", (void*)iarray, &nitems,
            &isize, "double", phasta_iotype);
        nitems = (*nshg)*(*numVars);
        phio_writedatablock(f_descriptor, "solution", (void*)(array1),
            &isize, "double", phasta_iotype );
    }
    field_flag++;

    for ( i = 0; i < nppp; i++) {
        // Write solution field ...
        isize = (*nshg)*(*numVars);
        nitems = 3;
        iarray[ 0 ] = (*nshg);
        iarray[ 1 ] = (*numVars);
        iarray[ 2 ] = (*stepno);
        phio_writeheader(f_descriptor, "time derivative of solution",
            (void*)iarray, &nitems, &isize, "double", phasta_iotype);
        nitems = (*nshg)*(*numVars);
        phio_writedatablock(f_descriptor, "time derivative of solution",
            (void*)(array2), &isize, "double", phasta_iotype );
    }
    field_flag++;

    if (field_flag==nfields){
      phio_closefile(f_descriptor);
      if (*pid==0) {
        printf("\n");
      }
    }
}

void
Write_Error(  int* pid,
              int* stepno,
              int* nshg,
              int* numVars,
              double* array1 ) {
    char fname[255];
    char rfile[60];
    int irstou;
    int magic_number = 362436;
    int* mptr = &magic_number;
    double version=0.0;
    int isize, nitems;
    int iarray[10];
    int nfiles;
    int nfields;
    int numparts;
    int irank;
    int nprocs;

    nfiles = outpar.nsynciofiles;
    nfields = outpar.nsynciofieldswriterestart;
    numparts = workfc.numpe;
    irank = *pid; //workfc.myrank;
    nprocs = workfc.numpe;

    // Calculate number of parts each  proc deal with and where it start and end ...
    int nppp = numparts/nprocs;// nppp : Number of parts per proc ...
    int startpart = irank * nppp +1;// Part id from which I (myrank) start ...
    int endpart = startpart + nppp - 1;// Part id to which I (myrank) end ...

    field_flag++;

    int i;
    for ( i = 0; i < nppp; i++  ) {

        if(*pid==0) {
          printf("\n");
          printf("The %d/%d th field to be written is 'errors'\n",field_flag,nfields);
        }

        isize = (*nshg)*(*numVars);
        nitems = 3;
        iarray[ 0 ] = (*nshg);
        iarray[ 1 ] = (*numVars);
        iarray[ 2 ] = (*stepno);

        phio_writeheader(f_descriptor, "errors", (void*)iarray, &nitems,
            &isize, "double", phasta_iotype);

        phio_writedatablock(f_descriptor, "errors", (void*)array1, &isize,
            "double", phasta_iotype );
    }
    if (field_flag==nfields){
      phio_closefile(f_descriptor);
      if (*pid==0) {
        printf("Last field %d 'errors' finished! \n",nfields);
        printf("\n");
      }
    }
}

void
Write_Displ(  int* pid,
              int* stepno,
              int* nshg,
              int* numVars,
              double* array1 ) {
  fprintf(stderr, "This function is dead...exiting\n");
  exit(1);
}

void
Write_Field(  int *pid,
              char* filemode,
              char* fieldtag,
              int* tagsize,
              void* array,
              char* arraytype,
              int* nshg,
              int* numvars,
              int* stepno) {
    char *fieldlabel = (char *)malloc((*tagsize+1)*sizeof(char));
    strncpy(fieldlabel, fieldtag, *tagsize);
    fieldlabel[*tagsize] = '\0';

    int irstou;
    int magic_number = 362436;
    int* mptr = &magic_number;
    double version=0.0;
    int isize, nitems;
    int iarray[10];

    char fmode[10];
    if(!strncmp(filemode,"w",1))
      strcpy(fmode,"write");
    else // default is append
      strcpy(fmode,"append");

    char datatype[10];
    if(!strncmp(arraytype,"i",1))
      strcpy(datatype,"int");
    else // default is double
      strcpy(datatype,"double");

    int nfiles;
    int nfields;
    int numparts;
    int irank;
    int nprocs;

    nfiles = outpar.nsynciofiles;
    nfields = outpar.nsynciofieldswriterestart;
    numparts = workfc.numpe;
    irank = *pid; //workfc.myrank;
    nprocs = workfc.numpe;

    // Calculate number of parts each  proc deal with and where it start and end ...
    int nppp = numparts/nprocs;// nppp : Number of parts per proc ...
    int startpart = irank * nppp +1;// Part id from which I (myrank) start ...
    int endpart = startpart + nppp - 1;// Part id to which I (myrank) end ...

    strncpy(fieldlabel, fieldtag, *tagsize);

    field_flag++;
    if(*pid==0) {
      printf("\n");
      printf("The %d/%d th field to be written is '%s'\n",field_flag,nfields,fieldlabel);
    }

    int i;
    for ( i = 0; i < nppp; i++  ) {
        // Write solution field ...
        isize = (*nshg)*(*numvars);
        nitems = 3;
        iarray[ 0 ] = (*nshg);
        iarray[ 1 ] = (*numvars);
        iarray[ 2 ] = (*stepno);

        phio_writeheader(f_descriptor, fieldlabel, (void*)iarray, &nitems,
            &isize, datatype, phasta_iotype);
        nitems = (*nshg)*(*numvars);
        phio_writedatablock(f_descriptor, fieldlabel, array, &isize,
            datatype, phasta_iotype );
    }
    if (field_flag==nfields){
      phio_closefile(f_descriptor);
      if (*pid==0) {
        printf("Last field %d '%s' finished! \n",nfields, fieldtag);
        printf("\n");
      }
    }
    free(fieldlabel);
}

void
Write_PhAvg2( int* pid,
              char* filemode,
              char* fieldtag,
              int* tagsize,
              int* iphase,
              int* nphasesincycle,
              void* array,
              char* arraytype,
              int* nshg,
              int* numvars,
              int* stepno) {
    int addtagsize=0; // phase number is added to the name of the field
    if(*iphase<10)
      addtagsize=1;
    else if(*iphase<100)
      addtagsize=2;
    else if(*iphase<1000)
      addtagsize=3;

    int tagsize2;
    tagsize2=*tagsize+addtagsize;

    char *fieldlabel = (char *)malloc((tagsize2+1)*sizeof(char));
    strncpy(fieldlabel, fieldtag, *tagsize);
    fieldlabel[tagsize2] = '\0';

    char straddtagsize[10];
    sprintf(straddtagsize,"%d",*iphase);

    if(*iphase<10) {
      fieldlabel[tagsize2-1]=straddtagsize[0];
    }
    else if(*iphase<100) {
      fieldlabel[tagsize2-2]=straddtagsize[0];
      fieldlabel[tagsize2-1]=straddtagsize[1];
    }
    else if(*iphase<1000) {
      fieldlabel[tagsize2-3]=straddtagsize[0];
      fieldlabel[tagsize2-2]=straddtagsize[1];
      fieldlabel[tagsize2-1]=straddtagsize[2];
    }

    int irstou;
    int magic_number = 362436;
    int* mptr = &magic_number;
    double version=0.0;
    int isize, nitems;
    int iarray[10];

    char fmode[10];
    if(!strncmp(filemode,"w",1))
      strcpy(fmode,"write");
    else // default is append
      strcpy(fmode,"append");

    char datatype[10];
    if(!strncmp(arraytype,"i",1))
      strcpy(datatype,"int");
    else // default is double
      strcpy(datatype,"double");

    int nfiles;
    int nfields;
    int numparts;
    int irank;
    int nprocs;

    nfiles = outpar.nsynciofiles;
    nfields = outpar.nsynciofieldswriterestart;
    numparts = workfc.numpe;
    irank = *pid; //workfc.myrank;
    nprocs = workfc.numpe;

    // Calculate number of parts each  proc deal with and where it start and end ...
    int nppp = numparts/nprocs;// nppp : Number of parts per proc ...
    int startpart = irank * nppp +1;// Part id from which I (myrank) start ...
    int endpart = startpart + nppp - 1;// Part id to which I (myrank) end ...

    field_flag++;
    if(*pid==0) {
      printf("\n");
      printf("The %d/%d th field to be written is '%s'\n",field_flag,nfields,fieldlabel);
    }

    int i;
    for ( i = 0; i < nppp; i++  ) {
        // Write solution field ...
        isize = (*nshg)*(*numvars);
        nitems = 3;
        iarray[ 0 ] = (*nshg);
        iarray[ 1 ] = (*numvars);
        iarray[ 2 ] = (*stepno);
        phio_writeheader(f_descriptor, fieldlabel, (void*)iarray, &nitems,
            &isize, "double", phasta_iotype);
        nitems = (*nshg)*(*numvars);
        phio_writedatablock(f_descriptor, fieldlabel, array, &isize,
            "double", phasta_iotype );
    }
    if (field_flag==nfields){
      phio_closefile(f_descriptor);
      if (*pid==0) {
        printf("\n");
      }
    }
    free(fieldlabel);
}
