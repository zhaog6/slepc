/*
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   SLEPc - Scalable Library for Eigenvalue Problem Computations
   Copyright (c) 2002-, Universitat Politecnica de Valencia, Spain

   This file is part of SLEPc.
   SLEPc is distributed under a 2-clause BSD license (see LICENSE).
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/
/*
   MFN routines related to monitors
*/

#include <slepc/private/mfnimpl.h>   /*I "slepcmfn.h" I*/
#include <petscdraw.h>

PetscErrorCode MFNMonitorLGCreate(MPI_Comm comm,const char host[],const char label[],const char metric[],PetscInt l,const char *names[],int x,int y,int m,int n,PetscDrawLG *lgctx)
{
  PetscDraw      draw;
  PetscDrawAxis  axis;
  PetscDrawLG    lg;

  PetscFunctionBegin;
  PetscCall(PetscDrawCreate(comm,host,label,x,y,m,n,&draw));
  PetscCall(PetscDrawSetFromOptions(draw));
  PetscCall(PetscDrawLGCreate(draw,l,&lg));
  if (names) PetscCall(PetscDrawLGSetLegend(lg,names));
  PetscCall(PetscDrawLGSetFromOptions(lg));
  PetscCall(PetscDrawLGGetAxis(lg,&axis));
  PetscCall(PetscDrawAxisSetLabels(axis,"Convergence","Iteration",metric));
  PetscCall(PetscDrawDestroy(&draw));
  *lgctx = lg;
  PetscFunctionReturn(0);
}

/*
   Runs the user provided monitor routines, if any.
*/
PetscErrorCode MFNMonitor(MFN mfn,PetscInt it,PetscReal errest)
{
  PetscInt       i,n = mfn->numbermonitors;

  PetscFunctionBegin;
  for (i=0;i<n;i++) PetscCall((*mfn->monitor[i])(mfn,it,errest,mfn->monitorcontext[i]));
  PetscFunctionReturn(0);
}

/*@C
   MFNMonitorSet - Sets an ADDITIONAL function to be called at every
   iteration to monitor convergence.

   Logically Collective on mfn

   Input Parameters:
+  mfn     - matrix function context obtained from MFNCreate()
.  monitor - pointer to function (if this is NULL, it turns off monitoring)
.  mctx    - [optional] context for private data for the
             monitor routine (use NULL if no context is desired)
-  monitordestroy - [optional] routine that frees monitor context (may be NULL)

   Calling Sequence of monitor:
$   monitor(MFN mfn,int its,PetscReal errest,void *mctx)

+  mfn    - matrix function context obtained from MFNCreate()
.  its    - iteration number
.  errest - error estimate
-  mctx   - optional monitoring context, as set by MFNMonitorSet()

   Options Database Keys:
+    -mfn_monitor - print the error estimate
.    -mfn_monitor draw::draw_lg - sets line graph monitor for the error estimate
-    -mfn_monitor_cancel - cancels all monitors that have been hardwired into
      a code by calls to MFNMonitorSet(), but does not cancel those set via
      the options database.

   Notes:
   Several different monitoring routines may be set by calling
   MFNMonitorSet() multiple times; all will be called in the
   order in which they were set.

   Level: intermediate

.seealso: MFNMonitorCancel()
@*/
PetscErrorCode MFNMonitorSet(MFN mfn,PetscErrorCode (*monitor)(MFN,PetscInt,PetscReal,void*),void *mctx,PetscErrorCode (*monitordestroy)(void**))
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mfn,MFN_CLASSID,1);
  PetscCheck(mfn->numbermonitors<MAXMFNMONITORS,PetscObjectComm((PetscObject)mfn),PETSC_ERR_ARG_OUTOFRANGE,"Too many MFN monitors set");
  mfn->monitor[mfn->numbermonitors]           = monitor;
  mfn->monitorcontext[mfn->numbermonitors]    = (void*)mctx;
  mfn->monitordestroy[mfn->numbermonitors++]  = monitordestroy;
  PetscFunctionReturn(0);
}

/*@
   MFNMonitorCancel - Clears all monitors for an MFN object.

   Logically Collective on mfn

   Input Parameters:
.  mfn - matrix function context obtained from MFNCreate()

   Options Database Key:
.    -mfn_monitor_cancel - cancels all monitors that have been hardwired
      into a code by calls to MFNMonitorSet(),
      but does not cancel those set via the options database.

   Level: intermediate

.seealso: MFNMonitorSet()
@*/
PetscErrorCode MFNMonitorCancel(MFN mfn)
{
  PetscInt       i;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mfn,MFN_CLASSID,1);
  for (i=0; i<mfn->numbermonitors; i++) {
    if (mfn->monitordestroy[i]) PetscCall((*mfn->monitordestroy[i])(&mfn->monitorcontext[i]));
  }
  mfn->numbermonitors = 0;
  PetscFunctionReturn(0);
}

/*@C
   MFNGetMonitorContext - Gets the monitor context, as set by
   MFNMonitorSet() for the FIRST monitor only.

   Not Collective

   Input Parameter:
.  mfn - matrix function context obtained from MFNCreate()

   Output Parameter:
.  ctx - monitor context

   Level: intermediate

.seealso: MFNMonitorSet()
@*/
PetscErrorCode MFNGetMonitorContext(MFN mfn,void *ctx)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(mfn,MFN_CLASSID,1);
  *(void**)ctx = mfn->monitorcontext[0];
  PetscFunctionReturn(0);
}

/*@C
   MFNMonitorDefault - Print the error estimate of the current approximation at each
   iteration of the matrix function solver.

   Collective on mfn

   Input Parameters:
+  mfn    - matrix function context
.  its    - iteration number
.  errest - error estimate
-  vf     - viewer and format for monitoring

   Options Database Key:
.  -mfn_monitor - activates MFNMonitorDefault()

   Level: intermediate

.seealso: MFNMonitorSet()
@*/
PetscErrorCode MFNMonitorDefault(MFN mfn,PetscInt its,PetscReal errest,PetscViewerAndFormat *vf)
{
  PetscViewer    viewer = vf->viewer;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mfn,MFN_CLASSID,1);
  PetscValidHeaderSpecific(viewer,PETSC_VIEWER_CLASSID,4);
  PetscCall(PetscViewerPushFormat(viewer,vf->format));
  PetscCall(PetscViewerASCIIAddTab(viewer,((PetscObject)mfn)->tablevel));
  if (its == 1 && ((PetscObject)mfn)->prefix) PetscCall(PetscViewerASCIIPrintf(viewer,"  Error estimates for %s solve.\n",((PetscObject)mfn)->prefix));
  PetscCall(PetscViewerASCIIPrintf(viewer,"%3" PetscInt_FMT " MFN Error estimate %14.12e\n",its,(double)errest));
  PetscCall(PetscViewerASCIISubtractTab(viewer,((PetscObject)mfn)->tablevel));
  PetscCall(PetscViewerPopFormat(viewer));
  PetscFunctionReturn(0);
}

/*@C
   MFNMonitorDefaultDrawLG - Plots the error estimate of the current approximation at each
   iteration of the matrix function solver.

   Collective on mfn

   Input Parameters:
+  mfn    - matrix function context
.  its    - iteration number
.  errest - error estimate
-  vf     - viewer and format for monitoring

   Options Database Key:
.  -mfn_monitor draw::draw_lg - activates MFNMonitorDefaultDrawLG()

   Level: intermediate

.seealso: MFNMonitorSet()
@*/
PetscErrorCode MFNMonitorDefaultDrawLG(MFN mfn,PetscInt its,PetscReal errest,PetscViewerAndFormat *vf)
{
  PetscViewer    viewer = vf->viewer;
  PetscDrawLG    lg = vf->lg;
  PetscReal      x,y;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(mfn,MFN_CLASSID,1);
  PetscValidHeaderSpecific(viewer,PETSC_VIEWER_CLASSID,4);
  PetscValidHeaderSpecific(lg,PETSC_DRAWLG_CLASSID,4);
  PetscCall(PetscViewerPushFormat(viewer,vf->format));
  if (its==1) {
    PetscCall(PetscDrawLGReset(lg));
    PetscCall(PetscDrawLGSetDimension(lg,1));
    PetscCall(PetscDrawLGSetLimits(lg,1,1.0,PetscLog10Real(mfn->tol)-2,0.0));
  }
  x = (PetscReal)its;
  if (errest > 0.0) y = PetscLog10Real(errest);
  else y = 0.0;
  PetscCall(PetscDrawLGAddPoint(lg,&x,&y));
  if (its <= 20 || !(its % 5) || mfn->reason) {
    PetscCall(PetscDrawLGDraw(lg));
    PetscCall(PetscDrawLGSave(lg));
  }
  PetscCall(PetscViewerPopFormat(viewer));
  PetscFunctionReturn(0);
}

/*@C
   MFNMonitorDefaultDrawLGCreate - Creates the plotter for the error estimate.

   Collective on viewer

   Input Parameters:
+  viewer - the viewer
.  format - the viewer format
-  ctx    - an optional user context

   Output Parameter:
.  vf     - the viewer and format context

   Level: intermediate

.seealso: MFNMonitorSet()
@*/
PetscErrorCode MFNMonitorDefaultDrawLGCreate(PetscViewer viewer,PetscViewerFormat format,void *ctx,PetscViewerAndFormat **vf)
{
  PetscFunctionBegin;
  PetscCall(PetscViewerAndFormatCreate(viewer,format,vf));
  (*vf)->data = ctx;
  PetscCall(MFNMonitorLGCreate(PetscObjectComm((PetscObject)viewer),NULL,"Error Estimate","Log Error Estimate",1,NULL,PETSC_DECIDE,PETSC_DECIDE,400,300,&(*vf)->lg));
  PetscFunctionReturn(0);
}
