#!/usr/bin/env python
import bs
import BSTemplates.sidl
import fileset

import os
import sys
import argtest

class PetscMake(bs.BS):
  def __init__(self, args = None):
    bs.BS.__init__(self, args)
    self.defineHelp()
    self.defineDirectories()
    self.defineFileSets()
    self.defineTargets()

  def defineHelp(self):
    bs.argDB.setHelp('PYTHON_INCLUDE', 'The directory in which the Python headers were installed (like Python.h)')
    bs.argDB.setTester('PYTHON_INCLUDE',argtest.DirectoryTester())

    bs.argDB.setHelp('SIDLRUNTIME_DIR', 'The directory in which the SIDL runtime was installed')
    bs.argDB.setTester('SIDLRUNTIME_DIR',argtest.DirectoryNotNoneTester())

  def defineDirectories(self):
    self.directories['sidl'] = os.path.abspath('sidl')

  def defineFileSets(self):
    self.filesets['sidl'] = fileset.ExtensionFileSet(self.directories['sidl'], '.sidl')

  def defineTargets(self):
    babelDefaults = BSTemplates.sidl.CompileDefaults('bs', self.filesets['sidl'])
    babelDefaults.addServerLanguage('C++')
    babelDefaults.addClientLanguage('C++')
    babelDefaults.addClientLanguage('Python')

    self.targets['sidl']    = babelDefaults.getSIDLTarget()
    self.targets['compile'] = babelDefaults.getCompileTarget()
    self.targets['default'] = self.targets['compile']

if __name__ ==  '__main__': PetscMake(sys.argv[1:]).main()
