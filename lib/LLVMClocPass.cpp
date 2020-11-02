
// Copyright Marcin Copik, 2020

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>

#include <cxxabi.h>
#include <iostream>
#include <map>
#include <string>
#include <tuple>


namespace cloc {

  struct CLOCPass : public llvm::ModulePass
  {
    static char ID;

    CLOCPass():
      ModulePass(ID)
    {}

    std::string demangle(llvm::StringRef name)
    {
      int status = 0;
      char * demangled_name =
          abi::__cxa_demangle(name.data(), 0, 0, &status);
      std::string str_name; if(status == 0)
          str_name = demangled_name;
      else if(status == -2)
          str_name = name;
      else
          assert(false);
      free( static_cast<void*>(demangled_name) );
      return str_name;
    }
    
    void getAnalysisUsage(llvm::AnalysisUsage & AU) const override
    {
      ModulePass::getAnalysisUsage(AU);
      AU.setPreservesAll();
    }

    bool runOnModule(llvm::Module & m) override
    {
      std::map<std::string, llvm::Function*> functions;
      for(llvm::Function & f : m)
      {
        if(f.isDeclaration())
          continue;
        auto demangled_name = demangle(f.getName());
        functions.insert(std::make_pair(demangled_name, &f));
      }
      std::cerr << "function"<< "\t\t" << "lines of code" << "\t\t" << "file" << '\n';
      std::cerr << "--------------------------------------------------\n";
      for(const auto & [key, value] : functions) {
        std::cerr << key << "\t\t";
        auto [locs, file] = runOnFunction(*value);
        std::cerr << locs << "\t\t\t" << file << '\n';
      }
      return false;
    }

    std::tuple<size_t, std::string> runOnFunction(llvm::Function & f)
    { 
      llvm::DISubprogram * prog = f.getSubprogram();
      size_t begin_line = prog->getLine();
      //std::cerr << begin_line;
      size_t highest_line = 0;
      for(llvm::BasicBlock & bb : f)
        for(llvm::Instruction & i : bb) {
          auto dbg_loc = i.getDebugLoc();
          if(dbg_loc)
            highest_line = std::max(highest_line, static_cast<size_t>(dbg_loc.getLine()));
        }
      //std::cerr << begin_line << "\t" << highest_line << "\n";
      return std::make_tuple(highest_line - begin_line + 1, prog->getFilename());
    }
  };
}

char cloc::CLOCPass::ID = 0;
static llvm::RegisterPass<cloc::CLOCPass> register_pass(
  "cloc",
  "Count lines of code (LOC) for each function.",
  true /* Only looks at CFG */,
  true /* Analysis Pass */
);

