#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <cxxabi.h>

using namespace llvm;
using namespace legacy;

namespace {
/**
 *  Change all occurences of `operator new` with `custom_new` and all occurences
 *  of `operator delete` with `custom_delete`
 *  @note only the basic `operator new` is considered
 */
struct CustomNewDelete : public BasicBlockPass {
  static char ID;
  /** Mangled custom_new function name. */
  static StringRef CUSTOM_NEW_NAME;
  static StringRef DEMANGLED_OP_NEW;
  /** Mangled custom_delete function name. */
  static StringRef CUSTOM_DELETE_NAME;
  static StringRef DEMANGLED_OP_DELETE;
  /** The declaration of custom_new inside of the module. */
  static Function* CUSTOM_NEW_FUNC;
  /** The declaration of custom_delete inside of the module. */
  static Function* CUSTOM_DELETE_FUNC;

  CustomNewDelete() : BasicBlockPass(ID) {}

  StringRef getDemangledName(StringRef name) {
    int status = -1;
    char* demangledName = abi::__cxa_demangle(
      name.str().c_str(), NULL, NULL, &status
    );
    StringRef s(demangledName ? demangledName : "");
    return s;
  }

  /**
   *  Insert a call to `custom_new` after each `operator new` call and
   *  change all `operator delete` calls to `custom_delete`.
   *  @param bb the BasicBlock in which the call to custom new and
   *            delete are inserted
   *  @param insts the vector in which all calls to operator new are stored
   */
  void addCustomNewAndDeleteCalls(BasicBlock& bb,
                                  std::vector<CallInst*>& insts) {
    for (auto& inst : bb) {
      if (isa<CallInst>(inst)) {
        CallInst& ci = cast<CallInst>(inst);
        auto func = ci.getCalledFunction();
        if (func) {
          StringRef name = getDemangledName(func->getName());
          IRBuilder<> builder(&ci);
          if (name == DEMANGLED_OP_NEW) {
            // replace the call to operator new with custom_new
            // but make sure the first argument of operator new
            // is copied into custom new as well!
            ci.replaceAllUsesWith(
              builder.CreateCall(CUSTOM_NEW_FUNC,
                                 { ci.getOperand(0), builder.getInt64(0) })
            );
            // save call instruction to remove it later
            insts.push_back(&ci);
          } else if (name == DEMANGLED_OP_DELETE) {
            // we are processing an operator delete call
            // because custom_delete and operator delete take the same
            // type and number of arguments, we can just change the
            // function called in this case call custom_delete
            ci.setCalledFunction(CUSTOM_DELETE_FUNC);
          }
        }
      }
    }
  }

  using BasicBlockPass::doInitialization;
  bool doInitialization(Module& mod) override {
    LLVMContext& context = mod.getContext();

    // custom_new type definition: void* custom_new(size_t, size_t)
    FunctionType* customNewType = FunctionType::get(
      PointerType::getUnqual(Type::getVoidTy(context)),
      { Type::getInt64Ty(context), Type::getInt64Ty(context) },
      false
    );
    // add the declaration of custom_new into the module
    // because we will link it later
    mod.getOrInsertFunction(CUSTOM_NEW_NAME, customNewType);
    CUSTOM_NEW_FUNC = mod.getFunction(CUSTOM_NEW_NAME);

    // custom_delete type definition: void* custom_delete(void*)
    FunctionType* customDeleteType = FunctionType::get(
      PointerType::getUnqual(Type::getVoidTy(context)),
      { Type::getVoidTy(context) },
      false
    );
    // same for custom_delete
    mod.getOrInsertFunction(CUSTOM_DELETE_NAME, customDeleteType);
    CUSTOM_DELETE_FUNC = mod.getFunction(CUSTOM_DELETE_NAME);
    return true;
  }

  bool runOnBasicBlock(BasicBlock& bb) override {
    Module* mod = bb.getParent()->getParent();
    const DataLayout& dataLayout = mod->getDataLayout();
    // all calls to operator new will be saved in this vector
    std::vector<CallInst*> insts;
    addCustomNewAndDeleteCalls(bb, insts);
    // remove all calls to operator new
    // because we inserted custom_new after each operator new
    while (insts.size() > 0) {
      insts.back()->removeFromParent();
      insts.pop_back();
    }

    IRBuilder<> builder(mod->getContext());
    // set to the last custom_new call encountered
    CallInst* lastCustomNewCall = nullptr;
    // set whenever a call to custom_new is encountered
    bool considerBitCast = false;
    for (auto& inst : bb) {
      if (isa<CallInst>(inst)) {
        CallInst& ci = cast<CallInst>(inst);
        auto func = ci.getCalledFunction();
        if (func) {
          StringRef name = func->getName();
          if (name == CUSTOM_NEW_NAME) {
            // processing a custom_new means that the next bitcast
            // will give us the alignment
            lastCustomNewCall = &ci;
            considerBitCast = true;
          }
        }
      } else if (considerBitCast && isa<BitCastInst>(inst)) {
        // the last call instruction was a call to custom_new
        // therefore this bitcast holds the type and the alignment
        // of the type which is allocated
        BitCastInst& bci = cast<BitCastInst>(inst);
        PointerType& pt = cast<PointerType>(*bci.getDestTy());
        Type* type = pt.getPointerElementType();
        size_t alignment = dataLayout.getPrefTypeAlignment(type);
        lastCustomNewCall->setOperand(1, builder.getInt64(alignment));
        considerBitCast = false;
      }
    }
    return true;
  }
}; // end of struct New
}  // end of anonymous namespace

char CustomNewDelete::ID = 0;
StringRef CustomNewDelete::CUSTOM_NEW_NAME = "_Z10custom_newmm";
StringRef CustomNewDelete::DEMANGLED_OP_NEW = "operator new(unsigned long)";
StringRef CustomNewDelete::CUSTOM_DELETE_NAME = "_Z13custom_deletePv";
StringRef CustomNewDelete::DEMANGLED_OP_DELETE = "operator delete(void*)";
Function* CustomNewDelete::CUSTOM_NEW_FUNC = nullptr;
Function* CustomNewDelete::CUSTOM_DELETE_FUNC = nullptr;

static RegisterPass<CustomNewDelete> X("custom new delete",
                                       "CustomNewDelete Pass",
                                       false /* Only looks at CFG */,
                                       false /* Analysis Pass */);

static void registerMyPass(const PassManagerBuilder &,
                           PassManagerBase &PM) {
  PM.add(new CustomNewDelete());
}
static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerMyPass);
