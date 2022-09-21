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

class Slicot(package.Package):

  def __init__(self,argdb,log):
    package.Package.__init__(self,argdb,log)
    self.packagename    = 'slicot'
    self.installable    = True
    self.version        = '4.5'
    self.archive        = 'slicot45.tar.gz'
    self.url            = 'http://slicot.org/objects/software/shared/'+self.archive
    self.supportsscalar = ['real']
    self.fortran        = True
    self.ProcessArgs(argdb)


  def Check(self,slepcconf,slepcvars,petsc,archdir):
    functions = ['sb03od','sb03md']
    libs = self.packagelibs if self.packagelibs else [['-lslicot']]

    if self.packagedir:
      dirs = [os.path.join(self.packagedir,'lib'),self.packagedir,os.path.join(self.packagedir,'lib64')]
    else:
      dirs = self.GenerateGuesses('slicot',archdir)

    self.FortranLib(slepcconf,slepcvars,dirs,libs,functions)


  def DownloadAndInstall(self,slepcconf,slepcvars,slepc,petsc,archdir,prefixdir):
    externdir = slepc.GetExternalPackagesDir(archdir)
    builddir  = self.Download(externdir,slepc.downloaddir)
    libname = 'libslicot.a'

    # Makefile
    cont  = 'FORTRAN   = '+petsc.fc+'\n'
    cont += 'OPTS      = '+petsc.getFFlags()+'\n'
    cont += 'ARCH      = '+petsc.ar+'\n'
    cont += 'ARCHFLAGS = '+petsc.ar_flags+'\n'
    cont += 'SLICOTLIB = ../'+libname+'\n'
    self.WriteMakefile('make.inc',builddir,cont)

    # Build package
    target = 'lib'
    (result,output) = self.RunCommand('cd '+builddir+'&&'+petsc.make+' clean &&'+petsc.make+' '+target)
    if result:
      self.log.Exit('Installation of SLICOT failed')

    # Move files
    incdir,libdir = slepc.CreatePrefixDirs(prefixdir)
    os.rename(os.path.join(builddir,libname),os.path.join(libdir,libname))

    # Check build
    functions = ['sb03od']
    libs = [['-lslicot']]
    dirs = [libdir]
    self.FortranLib(slepcconf,slepcvars,dirs,libs,functions)

