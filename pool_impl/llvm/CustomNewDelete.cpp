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
#include <algorithm>
#include <vector>
#include <map>
#include <cxxabi.h>

using namespace llvm;
using namespace legacy;
using std::vector;
using std::map;
using std::find;

namespace {
/**
 *  Change all occurences of `operator new` with `custom_new` and all occurences
 *  of `operator delete` with `custom_delete`
 *  @note All versions of operator new and delete are considered.
 */
struct CustomNewDelete : public BasicBlockPass {
  static char ID;
  /** Mangled custom_new function name. */
  static const StringRef CUSTOM_NEW_NAME;
  /** Mangled custom_new_no_throw function name. */
  static const StringRef CUSTOM_NEW_NO_THROW_NAME;
  /** Mangled custom_delete function name. */
  static const StringRef CUSTOM_DELETE_NAME;
  /** The declaration of custom_new inside of the module. */
  static Function* CUSTOM_NEW_FUNC;
  /** The declaration of custom_new_no_throw inside of the module. */
  static Function* CUSTOM_NEW_NO_THROW_FUNC;
  /** The declaration of custom_delete inside of the module. */
  static Function* CUSTOM_DELETE_FUNC;
  /** The demangled names of all operator new functions that
      throw std::bad_alloc. */
  static const vector<StringRef> NEW_OPS;
  /** The demangled names of all operator new functions that
      do not throw std::bad_alloc. */
  static const vector<StringRef> NEW_NO_THROW_OPS;
  /** The demangled names of all operator delete functions. */
  static const vector<StringRef> DELETE_OPS;
  /** The demangled names of all operator delete functions which take
      an extra argument (i.e. std::nothrow) */
  static const vector<StringRef> DELETE_NO_THROW_OPS;
  /** A mapping from operator news to their `custom_new` correspondent. */
  static const map<StringRef, Function**> OP_TO_CUSTOM;

  /**
   *  @param name a demangled Function name
   *  @return whether `name` is in `NEW_OPS` or in `NEW_NO_THROW_OPS`.
   */
  static bool isNew(const StringRef& name) {
    return find(NEW_OPS.begin(), NEW_OPS.end(), name) != NEW_OPS.end()
      || find(NEW_NO_THROW_OPS.begin(), NEW_NO_THROW_OPS.end(), name)
        != NEW_NO_THROW_OPS.end();
  }

  /**
   *  @param name a demangled Function name
   *  @return whether `name` is in `DELETE_OPS` or in `DELETE_NO_THROW_OPS`.
   */
  static bool isDelete(const StringRef& name) {
    return find(DELETE_OPS.begin(), DELETE_OPS.end(), name)
        != DELETE_OPS.end()
      || find(DELETE_NO_THROW_OPS.begin(), DELETE_NO_THROW_OPS.end(), name)
        != DELETE_NO_THROW_OPS.end();
  }

  CustomNewDelete() : BasicBlockPass(ID) {}

  /**
   *  @param name a demangled Function name
   *  @return the demangled name of a LLVM function.
   */
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
   *  @note InvokeInst are considered as well.
   */
  void addCustomNewAndDeleteCalls(BasicBlock& bb,
                                  std::vector<Instruction*>& insts) {
    for (auto& inst : bb) {
      if (isa<CallInst>(inst)) {
        CallInst& ci = cast<CallInst>(inst);
        auto func = ci.getCalledFunction();
        if (func) {
          StringRef name = getDemangledName(func->getName());
          IRBuilder<> builder(&ci);
          if (isNew(name)) {
            // replace the call to operator new with custom_new
            // but make sure the first argument of operator new
            // is copied into custom new as well!
            ci.replaceAllUsesWith(
              builder.CreateCall(*OP_TO_CUSTOM.at(name),
                                 { ci.getOperand(0), builder.getInt64(0) })
            );
            // save call instruction to remove it later
            insts.push_back(&ci);
          } else if (isDelete(name)) {
            // we are processing an operator delete call
            // because custom_delete and operator delete take the same
            // type and number of arguments, we can just change the
            // function called in this case call custom_delete
            ci.setCalledFunction(CUSTOM_DELETE_FUNC);
          }
        }
      } else if (isa<InvokeInst>(inst)) {
          InvokeInst& ii = cast<InvokeInst>(inst);
          auto func = ii.getCalledFunction();
          if (func) {
            StringRef name = getDemangledName(func->getName());
            IRBuilder<> builder(&ii);
            if (isNew(name)) {
              InvokeInst* customNewInvoke =
                // InvokeInsts seem to not hold the type, therefore we will
                // assume the largest alignment possible
                builder.CreateInvoke(*OP_TO_CUSTOM.at(name),
                                     ii.getNormalDest(),
                                     ii.getUnwindDest(),
                                     { ii.getOperand(0),
                                       builder.getInt64(alignof(max_align_t)) });
              AttributeSet attrs;
              attrs.addAttribute(ii.getContext(), 0, Attribute::NoAlias);
              customNewInvoke->setAttributes(attrs);
              ii.replaceAllUsesWith(customNewInvoke);
              insts.push_back(&ii);
            } else if (isDelete(name)) {
              ii.setCalledFunction(CUSTOM_DELETE_FUNC);
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
      Type::getInt8PtrTy(context),
      { Type::getInt64Ty(context), Type::getInt64Ty(context) },
      false
    );
    // add the declaration of custom_new into the module
    // because it will be linked later
    mod.getOrInsertFunction(CUSTOM_NEW_NAME, customNewType);
    CUSTOM_NEW_FUNC = mod.getFunction(CUSTOM_NEW_NAME);

    // reuse customNewType because custom_new_no_throw has the same signature
    mod.getOrInsertFunction(CUSTOM_NEW_NO_THROW_NAME, customNewType);
    CUSTOM_NEW_NO_THROW_FUNC = mod.getFunction(CUSTOM_NEW_NO_THROW_NAME);

    // custom_delete type definition: void custom_delete(void*)
    FunctionType* customDeleteType = FunctionType::get(
      Type::getInt8PtrTy(context),
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
    std::vector<Instruction*> insts;
    addCustomNewAndDeleteCalls(bb, insts);
    // remove all calls to operator new/delete
    while (insts.size() > 0) {
      insts.back()->removeFromParent();
      insts.pop_back();
    }

    IRBuilder<> builder(mod->getContext());
    for (auto& inst : bb) {
      if (isa<CallInst>(inst)) {
        CallInst& ci = cast<CallInst>(inst);
        auto func = ci.getCalledFunction();
        if (func) {
          StringRef name = func->getName();
          if (name == CUSTOM_NEW_NAME) {
            Instruction* nextInst = inst.getNextNode();
            // processing a custom_new means that the next instruction
            // is a bitcast which will give us the alignment
            if (nextInst && isa<BitCastInst>(*nextInst)) {
              BitCastInst& bci = cast<BitCastInst>(*nextInst);
              PointerType& pt = cast<PointerType>(*bci.getDestTy());
              Type* type = pt.getPointerElementType();
              size_t alignment = dataLayout.getPrefTypeAlignment(type);
              ci.setOperand(1, builder.getInt64(alignment));
            } else {
              // if the BitCastInst was not present, we will choose
              // the largest alignment possible
              ci.setOperand(1, builder.getInt64(alignof(max_align_t)));
            }
          }
        }
      }
    }
    return true;
  }
}; // end of struct CustomNewDelete
}  // end of anonymous namespace

char CustomNewDelete::ID = 0;
const StringRef CustomNewDelete::CUSTOM_NEW_NAME = "_Z10custom_newmm";
const StringRef CustomNewDelete::CUSTOM_NEW_NO_THROW_NAME =
  "_Z19custom_new_no_throwmm";
const StringRef CustomNewDelete::CUSTOM_DELETE_NAME = "_Z13custom_deletePv";

Function* CustomNewDelete::CUSTOM_NEW_FUNC = nullptr;
Function* CustomNewDelete::CUSTOM_NEW_NO_THROW_FUNC = nullptr;
Function* CustomNewDelete::CUSTOM_DELETE_FUNC = nullptr;

const vector<StringRef> CustomNewDelete::NEW_OPS = {
  "operator new(unsigned long)",
  "operator new[](unsigned long)"
};
const vector<StringRef> CustomNewDelete::NEW_NO_THROW_OPS = {
  "operator new(unsigned long, std::nothrow_t const&)",
  "operator new[](unsigned long, std::nothrow_t const&)"
};
const vector<StringRef> CustomNewDelete::DELETE_OPS = {
  "operator delete(void*)",
  "operator delete[](void*)"
};
const vector<StringRef> CustomNewDelete::DELETE_NO_THROW_OPS = {
  "operator delete(void*, std::nothrow_t const&)",
  "operator delete[](void*, std::nothrow_t const&)"
};

const map<StringRef, Function**> CustomNewDelete::OP_TO_CUSTOM = {
  { NEW_OPS[0], &CUSTOM_NEW_FUNC },
  { NEW_OPS[1], &CUSTOM_NEW_FUNC },
  { NEW_NO_THROW_OPS[0], &CUSTOM_NEW_NO_THROW_FUNC },
  { NEW_NO_THROW_OPS[1], &CUSTOM_NEW_NO_THROW_FUNC }
};

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
