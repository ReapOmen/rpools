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

#include "common.h"

using namespace llvm;
using namespace legacy;
using namespace custom_pass;
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
  /** A mapping from operator news to their `custom_new` correspondent. */
  static const map<StringRef, Function**> OP_TO_CUSTOM;

  static size_t getAlignmentFromInst(llvm::Instruction* inst,
                                     const DataLayout& dataLayout) {
    size_t alignment = alignof(max_align_t);
    // check if the instruction is a bitcast
    // because it holds the type, therefore the alignment
    if (inst && isa<BitCastInst>(*inst)) {
      auto& bci = cast<BitCastInst>(*inst);
      auto& pt = cast<PointerType>(*bci.getDestTy());
      Type* type = pt.getPointerElementType();
      alignment = dataLayout.getPrefTypeAlignment(type);
    }
    return alignment;
  }

  CustomNewDelete() : BasicBlockPass(ID) {}

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
    mod.getOrInsertFunction(CUSTOM_DELETE_NAME, customDeleteType);
    CUSTOM_DELETE_FUNC = mod.getFunction(CUSTOM_DELETE_NAME);

    return true;
  }

  bool runOnBasicBlock(BasicBlock& bb) override {
    Module* mod = bb.getModule();
    const DataLayout& dataLayout = mod->getDataLayout();
    // all calls to operator new/delete will be saved in this vector
    std::vector<Instruction*> insts;
    for (auto& inst : bb) {
      if (isa<CallInst>(inst)) {
        auto& ci = cast<CallInst>(inst);
        auto func = ci.getCalledFunction();
        if (func && func->getName().data()) {
          std::string name = getDemangledName(func->getName().str());
          IRBuilder<> builder(&ci);
          if (isNew(name)) {
            // next inst might be a BitCast which holds the alignment of the
            // type being allocated
            size_t alignment = getAlignmentFromInst(inst.getNextNode(),
                                                    dataLayout);
            // replace the call to operator new with custom_new
            // but make sure the first argument of operator new
            // is copied into custom new as well!
            ci.replaceAllUsesWith(
              builder.CreateCall(*OP_TO_CUSTOM.at(name),
                                 { ci.getOperand(0),
                                   builder.getInt64(alignment) })
            );
            // save call instruction to remove it later
            insts.push_back(&ci);
          } else if (isDelete(name)) {
            // we are processing an operator delete call
            // because custom_delete and operator delete take the same
            // type and number of arguments, we can just change the
            // function called (in this case call custom_delete)
            ci.setCalledFunction(CUSTOM_DELETE_FUNC);
          }
        }
      } else if (isa<InvokeInst>(inst)) {
          auto& ii = cast<InvokeInst>(inst);
          auto func = ii.getCalledFunction();
          if (func) {
            std::string name = getDemangledName(func->getName().str());
            IRBuilder<> builder(&ii);
            if (isNew(name)) {
              // InvokeInsts seem to hold the BitCastInst which contains the
              // type being allocated in the NormalDest BasicBlock
              size_t alignment =
                getAlignmentFromInst(ii.getNormalDest()->getFirstNonPHI(),
                                     dataLayout);
              InvokeInst* customNewInvoke =
                builder.CreateInvoke(*OP_TO_CUSTOM.at(name),
                                     ii.getNormalDest(),
                                     ii.getUnwindDest(),
                                     { ii.getOperand(0),
                                       builder.getInt64(alignment) }
                );
              AttributeList attrs;
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
    // remove all calls to operator new/delete from the code
    // because custom_new/delete was inserted instead
    for (auto inst : insts) {
      inst->eraseFromParent();
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
