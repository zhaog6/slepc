#
#  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#  SLEPc - Scalable Library for Eigenvalue Problem Computations
#  Copyright (c) 2002-, Universitat Politecnica de Valencia, Spain
#
#  This file is part of SLEPc.
#  SLEPc is distributed under a 2-clause BSD license (see LICENSE).
#  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#

import os, log, package

class Trlan(package.Package):

  def __init__(self,argdb,log):
    package.Package.__init__(self,argdb,log)
    self.packagename    = 'trlan'
    self.installable    = True
    self.downloadable   = True
    self.version        = '201009'
    self.archive        = 'trlan-'+self.version+'.tar.gz'
    self.url            = 'http://slepc.upv.es/download/external/'+self.archive
    self.supportsscalar = ['real']
    self.fortran        = True
    self.ProcessArgs(argdb)

  def Check(self,slepcconf,slepcvars,petsc,archdir):
    functions = ['trlan77']
    if self.packagelibs:
      libs = self.packagelibs
    else:
      if petsc.mpiuni:
        libs = [['-ltrlan']]
      else:
        libs = [['-ltrlan_mpi']]

    if self.packagedir:
      dirs = [os.path.join(self.packagedir,'lib'),self.packagedir,os.path.join(self.packagedir,'lib64')]
    else:
      dirs = self.GenerateGuesses('TRLan',archdir)

    self.FortranLib(slepcconf,slepcvars,dirs,libs,functions)


  def DownloadAndInstall(self,slepcconf,slepcvars,slepc,petsc,archdir,prefixdir):
    externdir = slepc.GetExternalPackagesDir(archdir)
    builddir  = self.Download(externdir,slepc.downloaddir)

    # Makefile
    cont  = 'FC     = '+petsc.fc+'\n'
    cont += 'F90    = '+petsc.fc+'\n'
    cont += 'FFLAGS = '+petsc.getFFlags()+'\n'
    cont += 'SHELL  = /bin/sh\n'
    self.WriteMakefile('Make.inc',builddir,cont)

    # Build package
    if petsc.mpiuni:
      target = 'lib'
    else:
      target = 'plib'
    (result,output) = self.RunCommand('cd '+builddir+'&&'+petsc.make+' clean &&'+petsc.make+' '+target)
    if result:
      self.log.Exit('Installation of TRLAN failed')

    # Move files
    incdir,libdir = slepc.CreatePrefixDirs(prefixdir)
    if petsc.mpiuni:
      libName = 'libtrlan.a'
    else:
      libName = 'libtrlan_mpi.a'
    os.rename(os.path.join(builddir,libName),os.path.join(libdir,libName))

    # Check build
    functions = ['trlan77']
    if petsc.mpiuni:
      libs = [['-ltrlan']]
    else:
      libs = [['-ltrlan_mpi']]
    dirs = [libdir]
    self.FortranLib(slepcconf,slepcvars,dirs,libs,functions)

