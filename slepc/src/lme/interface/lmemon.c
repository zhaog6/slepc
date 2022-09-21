/*
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   SLEPc - Scalable Library for Eigenvalue Problem Computations
   Copyright (c) 2002-, Universitat Politecnica de Valencia, Spain

   This file is part of SLEPc.
   SLEPc is distributed under a 2-clause BSD license (see LICENSE).
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/
/*
   LME routines related to monitors
*/

#include <slepc/private/lmeimpl.h>   /*I "slepclme.h" I*/
#include <petscdraw.h>

PetscErrorCode LMEMonitorLGCreate(MPI_Comm comm,const char host[],const char label[],const char metric[],PetscInt l,const char *names[],int x,int y,int m,int n,PetscDrawLG *lgctx)
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
PetscErrorCode LMEMonitor(LME lme,PetscInt it,PetscReal errest)
{
  PetscInt       i,n = lme->numbermonitors;

  PetscFunctionBegin;
  for (i=0;i<n;i++) PetscCall((*lme->monitor[i])(lme,it,errest,lme->monitorcontext[i]));
  PetscFunctionReturn(0);
}

/*@C
   LMEMonitorSet - Sets an ADDITIONAL function to be called at every
   iteration to monitor convergence.

   Logically Collective on lme

   Input Parameters:
+  lme     - linear matrix equation solver context obtained from LMECreate()
.  monitor - pointer to function (if this is NULL, it turns off monitoring)
.  mctx    - [optional] context for private data for the
             monitor routine (use NULL if no context is desired)
-  monitordestroy - [optional] routine that frees monitor context (may be NULL)

   Calling Sequence of monitor:
$   monitor(LME lme,int its,PetscReal errest,void *mctx)

+  lme    - linear matrix equation solver context obtained from LMECreate()
.  its    - iteration number
.  errest - error estimate
-  mctx   - optional monitoring context, as set by LMEMonitorSet()

   Options Database Keys:
+    -lme_monitor - print the error estimate
.    -lme_monitor draw::draw_lg - sets line graph monitor for the error estimate
-    -lme_monitor_cancel - cancels all monitors that have been hardwired into
      a code by calls to LMEMonitorSet(), but does not cancel those set via
      the options database.

   Notes:
   Several different monitoring routines may be set by calling
   LMEMonitorSet() multiple times; all will be called in the
   order in which they were set.

   Level: intermediate

.seealso: LMEMonitorCancel()
@*/
PetscErrorCode LMEMonitorSet(LME lme,PetscErrorCode (*monitor)(LME,PetscInt,PetscReal,void*),void *mctx,PetscErrorCode (*monitordestroy)(void**))
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(lme,LME_CLASSID,1);
  PetscCheck(lme->numbermonitors<MAXLMEMONITORS,PetscObjectComm((PetscObject)lme),PETSC_ERR_ARG_OUTOFRANGE,"Too many LME monitors set");
  lme->monitor[lme->numbermonitors]           = monitor;
  lme->monitorcontext[lme->numbermonitors]    = (void*)mctx;
  lme->monitordestroy[lme->numbermonitors++]  = monitordestroy;
  PetscFunctionReturn(0);
}

/*@
   LMEMonitorCancel - Clears all monitors for an LME object.

   Logically Collective on lme

   Input Parameters:
.  lme - linear matrix equation solver context obtained from LMECreate()

   Options Database Key:
.    -lme_monitor_cancel - cancels all monitors that have been hardwired
      into a code by calls to LMEMonitorSet(),
      but does not cancel those set via the options database.

   Level: intermediate

.seealso: LMEMonitorSet()
@*/
PetscErrorCode LMEMonitorCancel(LME lme)
{
  PetscInt       i;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(lme,LME_CLASSID,1);
  for (i=0; i<lme->numbermonitors; i++) {
    if (lme->monitordestroy[i]) PetscCall((*lme->monitordestroy[i])(&lme->monitorcontext[i]));
  }
  lme->numbermonitors = 0;
  PetscFunctionReturn(0);
}

/*@C
   LMEGetMonitorContext - Gets the monitor context, as set by
   LMEMonitorSet() for the FIRST monitor only.

   Not Collective

   Input Parameter:
.  lme - linear matrix equation solver context obtained from LMECreate()

   Output Parameter:
.  ctx - monitor context

   Level: intermediate

.seealso: LMEMonitorSet()
@*/
PetscErrorCode LMEGetMonitorContext(LME lme,void *ctx)
{
  PetscFunctionBegin;
  PetscValidHeaderSpecific(lme,LME_CLASSID,1);
  *(void**)ctx = lme->monitorcontext[0];
  PetscFunctionReturn(0);
}

/*@C
   LMEMonitorDefault - Print the error estimate of the current approximation at each
   iteration of the linear matrix equation solver.

   Collective on lme

   Input Parameters:
+  lme    - linear matrix equation solver context
.  its    - iteration number
.  errest - error estimate
-  vf     - viewer and format for monitoring

   Options Database Key:
.  -lme_monitor - activates LMEMonitorDefault()

   Level: intermediate

.seealso: LMEMonitorSet()
@*/
PetscErrorCode LMEMonitorDefault(LME lme,PetscInt its,PetscReal errest,PetscViewerAndFormat *vf)
{
  PetscViewer    viewer = vf->viewer;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(lme,LME_CLASSID,1);
  PetscValidHeaderSpecific(viewer,PETSC_VIEWER_CLASSID,4);
  PetscCall(PetscViewerPushFormat(viewer,vf->format));
  PetscCall(PetscViewerASCIIAddTab(viewer,((PetscObject)lme)->tablevel));
  if (its == 1 && ((PetscObject)lme)->prefix) PetscCall(PetscViewerASCIIPrintf(viewer,"  Error estimates for %s solve.\n",((PetscObject)lme)->prefix));
  PetscCall(PetscViewerASCIIPrintf(viewer,"%3" PetscInt_FMT " LME Error estimate %14.12e\n",its,(double)errest));
  PetscCall(PetscViewerASCIISubtractTab(viewer,((PetscObject)lme)->tablevel));
  PetscCall(PetscViewerPopFormat(viewer));
  PetscFunctionReturn(0);
}

/*@C
   LMEMonitorDefaultDrawLG - Plots the error estimate of the current approximation at each
   iteration of the linear matrix equation solver.

   Collective on lme

   Input Parameters:
+  lme    - linear matrix equation solver context
.  its    - iteration number
.  errest - error estimate
-  vf     - viewer and format for monitoring

   Options Database Key:
.  -lme_monitor draw::draw_lg - activates LMEMonitorDefaultDrawLG()

   Level: intermediate

.seealso: LMEMonitorSet()
@*/
PetscErrorCode LMEMonitorDefaultDrawLG(LME lme,PetscInt its,PetscReal errest,PetscViewerAndFormat *vf)
{
  PetscViewer    viewer = vf->viewer;
  PetscDrawLG    lg = vf->lg;
  PetscReal      x,y;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(lme,LME_CLASSID,1);
  PetscValidHeaderSpecific(viewer,PETSC_VIEWER_CLASSID,4);
  PetscValidHeaderSpecific(lg,PETSC_DRAWLG_CLASSID,4);
  PetscCall(PetscViewerPushFormat(viewer,vf->format));
  if (its==1) {
    PetscCall(PetscDrawLGReset(lg));
    PetscCall(PetscDrawLGSetDimension(lg,1));
    PetscCall(PetscDrawLGSetLimits(lg,1,1.0,PetscLog10Real(lme->tol)-2,0.0));
  }
  x = (PetscReal)its;
  if (errest > 0.0) y = PetscLog10Real(errest);
  else y = 0.0;
  PetscCall(PetscDrawLGAddPoint(lg,&x,&y));
  if (its <= 20 || !(its % 5) || lme->reason) {
    PetscCall(PetscDrawLGDraw(lg));
    PetscCall(PetscDrawLGSave(lg));
  }
  PetscCall(PetscViewerPopFormat(viewer));
  PetscFunctionReturn(0);
}

/*@C
   LMEMonitorDefaultDrawLGCreate - Creates the plotter for the error estimate.

   Collective on viewer

   Input Parameters:
+  viewer - the viewer
.  format - the viewer format
-  ctx    - an optional user context

   Output Parameter:
.  vf     - the viewer and format context

   Level: intermediate

.seealso: LMEMonitorSet()
@*/
PetscErrorCode LMEMonitorDefaultDrawLGCreate(PetscViewer viewer,PetscViewerFormat format,void *ctx,PetscViewerAndFormat **vf)
{
  PetscFunctionBegin;
  PetscCall(PetscViewerAndFormatCreate(viewer,format,vf));
  (*vf)->data = ctx;
  PetscCall(LMEMonitorLGCreate(PetscObjectComm((PetscObject)viewer),NULL,"Error Estimate","Log Error Estimate",1,NULL,PETSC_DECIDE,PETSC_DECIDE,400,300,&(*vf)->lg));
  PetscFunctionReturn(0);
}
