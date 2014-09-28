// Copyright © 2011, Université catholique de Louvain
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// *  Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// *  Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef MOZART_MODSPACE_H
#define MOZART_MODSPACE_H

#include "../mozartcore.hh"

// Temporal include to use getpid()
#include <unistd.h>
// Temporal include to debug with gdb
#include <chrono>
#include <thread>

#ifndef MOZART_GENERATOR

namespace mozart {

namespace builtins {

///////////////////
// Space module //
///////////////////

class ModSpace: public Module {
public:
  ModSpace(): Module("Space") {}

  class New: public Builtin<New> {
  public:
    New(): Builtin("new") {}

    static void call(VM vm, In target, Out result) {
      // Print the process identifier and sleep for some time while the
      // debugger catches the process.
      std::cerr << "Current process identifier: " << getpid() << std::endl;
      std::chrono::seconds delay(15);
      std::cerr << "Will wait for gdb and resume in: " << delay.count() << "seconds\n";
      std::this_thread::sleep_for(delay);
      std::cerr << "resuming..." << std::endl;
      
      // Create the space
      Space* space = new (vm) Space(vm, vm->getCurrentSpace());
      
      std::cerr << "Impl(Space.new) created space: " << space << std::endl;

      // Create the thread {Proc Root}
      Thread *t = ozcalls::asyncOzCallForNewSpace(vm, space, target, *space->getRootVar());
      Thread::setDebugThread(t);
      t->dump();
      space->dumpStabilityInf();
      
      std::cerr << "Impl(Space.new) finished\n";
      
      // Create the reification of the space
      result = ReifiedSpace::build(vm, space);
    }
  };

  class Is: public Builtin<Is> {
  public:
    Is(): Builtin("is") {}

    static void call(VM vm, In value, Out result) {
      result = build(vm, SpaceLike(value).isSpace(vm));
    }
  };

  class Ask: public Builtin<Ask> {
  public:
    Ask(): Builtin("ask") {}

    static void call(VM vm, In space, Out result) {
      result = SpaceLike(space).askSpace(vm);
    }
  };

  class AskVerbose: public Builtin<AskVerbose> {
  public:
    AskVerbose(): Builtin("askVerbose") {}

    static void call(VM vm, In space, Out result) {
      result = SpaceLike(space).askVerboseSpace(vm);
    }
  };

  class Merge: public Builtin<Merge> {
  public:
    Merge(): Builtin("merge") {}

    static void call(VM vm, In space, Out result) {
      result = SpaceLike(space).mergeSpace(vm);
    }
  };

  class Clone: public Builtin<Clone> {
  public:
    Clone(): Builtin("clone") {}

    static void call(VM vm, In space, Out result) {
      result = SpaceLike(space).cloneSpace(vm);
    }
  };

  class Commit: public Builtin<Commit> {
  public:
    Commit(): Builtin("commit") {}

    static void call(VM vm, In space, In value) {
      return SpaceLike(space).commitSpace(vm, value);
    }
  };

  class Kill: public Builtin<Kill> {
  public:
    Kill(): Builtin("kill") {}

    static void call(VM vm, In space) {
      return SpaceLike(space).killSpace(vm);
    }
  };

  class Choose: public Builtin<Choose> {
  public:
    Choose(): Builtin("choose") {}

    static void call(VM vm, In alts, Out result) {
      auto alternatives = getArgument<nativeint>(vm, alts,
                                                 "integer");

      Space* space = vm->getCurrentSpace();

      if (space->isTopLevel()) {
        result = OptVar::build(vm);
      } else if (space->hasDistributor()) {
        raise(vm, "spaceDistributor");
      } else {
        ChooseDistributor* distributor =
          new (vm) ChooseDistributor(vm, space, alternatives);

        space->setDistributor(distributor);
        result.copy(vm, *distributor->getVar());
      }
    }
  };
};

}

}

#endif // MOZART_GENERATOR

#endif // MOZART_MODSPACE_H
