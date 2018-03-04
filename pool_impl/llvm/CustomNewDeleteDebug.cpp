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
#include <string>

#include "common.h"

using namespace llvm;
using namespace legacy;
using namespace custom_pass;
using std::vector;
using std::map;
using std::find;
using std::string;
using std::pair;
using std::tuple;
using std::get;

namespace {

struct TypeMetadata {
  size_t alignment = alignof(max_align_t);
  Value* name = nullptr;
  size_t size = 0;
  Value* funcName = nullptr;
  TypeMetadata() = default;
};

/**
 *  Change all occurences of `operator new` with `custom_new` and all occurences
 *  of `operator delete` with `custom_delete`
 *  @note All versions of operator new and delete are considered.
 */
struct CustomNewDeleteDebug : public BasicBlockPass {
  static char ID;
  static const string UNKNOWN_STR;
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
  static GlobalVariable* UNKNOWN_TYPE, * UNKNOWN_FUNC;

  /**
   * @param t_type the type whose name is returned
   * @return an approximation of the type's name.
   */
  static string getTypeName(const Type& t_type) {
    if (t_type.isPointerTy()) {
      return getTypeName(*t_type.getPointerElementType()) + "*";
    } else if (t_type.isStructTy()) {
      string s(t_type.getStructName());
      return s.substr(s.find(".") + 1);
    } else if (t_type.isFloatTy()) {
      return "float";
    } else if (t_type.isDoubleTy()) {
      return "double";
    }  else if (t_type.isArrayTy()) {
      return getTypeName(*t_type.getArrayElementType()) +
        "[" + std::to_string(t_type.getArrayNumElements()) + "]";
    } else if (t_type.isIntegerTy()) {
      return string("uint") + std::to_string(t_type.getIntegerBitWidth());
    }
    return UNKNOWN_STR;
  }

  /**
   * Inserts the given string into the module.
   * @param t_mod the module in which the string in inserted
   * @param t_str the string to be inserted
   * @param t_isFunc true if the string represents a function name or a
   *                 type name
   * @return a GlobalVariable which represents the given string in the module.
   */
  static GlobalVariable* getOrInsertStr(Module* t_mod, const string& t_str,
                                        bool t_isFunc) {
    auto str = ConstantDataArray::getString(t_mod->getContext(),
                                            t_str);
    string extendedStr = t_str;
    extendedStr += t_isFunc ? "_func_name__" : "_type_name__";
    t_mod->getOrInsertGlobal(extendedStr, str->getType());
    auto gv = t_mod->getGlobalVariable(extendedStr);
    gv->setConstant(true);
    gv->setAlignment(1);
    gv->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
    gv->setInitializer(str);
    return gv;
  }

  /**
   * @param t_builder a builder used to generate Values
   * @param t_gv a GlobalVariable to which a pointer is created
   * @return a GEP which points to the given GlobalVariable.
   */
  static Value* getGEP(IRBuilder<>& t_builder,
                       GlobalVariable* t_gv) {
    return t_builder.CreateInBoundsGEP(
            t_gv->getType()->getPointerElementType(),
            t_gv,
            { t_builder.getInt32(0),
              t_builder.getInt32(0) }
    );
  }

  /**
   * @param t_func the Function from which the name is extracted
   * @return the template parameters of the given function.
   */
  static string getNameFromFunc(Function* t_func) {
    string name = getDemangledName(t_func->getName());
    // if we can find allocate in the name of the function
    // that means that the function which calls new is an
    // allocator -> the name holds the type of allocator
    // therefore the name of the type being allocated
    // example:                 v--- the type              v-- allocator func
    // __gnu_cxx::new_allocator<std::_Rb_tree_node<int> >::allocate(...)
    int allocOffset = name.find("allocate");
    int firstAngleBracket = name.find("<");
    if (allocOffset > -1 && firstAngleBracket > -1) {
      string typeName(name.substr(firstAngleBracket + 1, // skip "<"
                                  // -4 to skip ">::"
                                  allocOffset - firstAngleBracket - 4));
      rtrim(typeName);
      return typeName;
    }
    return "";
  }

  /**
   * Extracts the alignment, the name and the size of the type out of the given
   * Instruction. This is possible only if the Instruction is a BitCastInst.
   * @param inst an instruction which might be a BitCast
   * @param dataLayout a DataLayout of the current Module
   * @param builder an IRBuilder
   * @return a TypeMetadata with reasonable values in case the given
   *         Instruction is not a BitCastInst.
   */
  static TypeMetadata getTypeMetadata(Instruction* inst,
                                      const DataLayout& dataLayout,
                                      IRBuilder<>& builder) {
    TypeMetadata tm;
    // check if the instruction is a bitcast
    // because it holds the type, therefore the alignment
    if (inst && isa<BitCastInst>(*inst)) {
      auto& bci = cast<BitCastInst>(*inst);
      auto& pt = cast<PointerType>(*bci.getDestTy());
      Type* type = pt.getPointerElementType();
      tm.alignment = dataLayout.getPrefTypeAlignment(type);
      tm.size = dataLayout.getTypeAllocSize(type);
      Function* func = inst->getFunction();
      string typeName = getNameFromFunc(func);
      typeName = typeName == "" ? getTypeName(*type) : typeName;
      tm.name = getGEP(builder, getOrInsertStr(inst->getModule(),
                                               typeName, false));
      string funcName = getDemangledName(func->getName());
      tm.funcName = getGEP(builder,
                           getOrInsertStr(inst->getModule(), funcName, true));
    } else {
      tm.name = getGEP(builder, UNKNOWN_TYPE);
      tm.funcName = getGEP(builder, UNKNOWN_FUNC);
    }
    return tm;
  }

  CustomNewDeleteDebug() : BasicBlockPass(ID) {}

  using BasicBlockPass::doInitialization;
  bool doInitialization(Module& mod) override {
    LLVMContext& context = mod.getContext();

    // custom_new type definition:
    // void* custom_new(size_t, size_t, char*, size_t, char*)
    FunctionType* customNewType = FunctionType::get(
      Type::getInt8PtrTy(context),
      { Type::getInt64Ty(context),
        Type::getInt64Ty(context),
        Type::getInt8PtrTy(context),
        Type::getInt64Ty(context),
        Type::getInt8PtrTy(context)},
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

    // create the UNKNOWN string
    UNKNOWN_TYPE = getOrInsertStr(&mod, UNKNOWN_STR, false);
    UNKNOWN_FUNC = getOrInsertStr(&mod, UNKNOWN_STR, true);

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
          if (isNew(name)) {
            IRBuilder<> builder(&ci);
            auto tm = getTypeMetadata(inst.getNextNode(), dataLayout, builder);
            ci.replaceAllUsesWith(
              builder.CreateCall(*OP_TO_CUSTOM.at(name),
                                 { ci.getOperand(0),
                                   builder.getInt64(tm.alignment),
                                   tm.name,
                                   builder.getInt64(tm.size),
                                   tm.funcName })
            );
            insts.push_back(&ci);
          } else if (isDelete(name)) {
            ci.setCalledFunction(CUSTOM_DELETE_FUNC);
          }
        }
      } else if (isa<InvokeInst>(inst)) {
          auto& ii = cast<InvokeInst>(inst);
          auto func = ii.getCalledFunction();
          if (func) {
            std::string name = getDemangledName(func->getName().str());
            if (isNew(name)) {
              IRBuilder<> builder(&ii);
              auto tm = getTypeMetadata(
                ii.getNormalDest()->getFirstNonPHI(),
                dataLayout,
                builder
              );
              InvokeInst* customNewInvoke =
                builder.CreateInvoke(*OP_TO_CUSTOM.at(name),
                                     ii.getNormalDest(),
                                     ii.getUnwindDest(),
                                     { ii.getOperand(0),
                                       builder.getInt64(tm.alignment),
                                       tm.name,
                                       builder.getInt64(tm.size),
                                       tm.funcName });
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
    for (auto inst : insts) {
      inst->eraseFromParent();
    }
    return true;
  }
}; // end of struct CustomNewDeleteDebug
}  // end of anonymous namespace

char CustomNewDeleteDebug::ID = 0;
const std::string CustomNewDeleteDebug::UNKNOWN_STR = "Unknown";
const StringRef CustomNewDeleteDebug::CUSTOM_NEW_NAME =
  "_Z10custom_newmmPKcmS0_";
const StringRef CustomNewDeleteDebug::CUSTOM_NEW_NO_THROW_NAME =
  "_Z19custom_new_no_throwmmPKcmS0_";
const StringRef CustomNewDeleteDebug::CUSTOM_DELETE_NAME =
  "_Z13custom_deletePv";

Function* CustomNewDeleteDebug::CUSTOM_NEW_FUNC = nullptr;
Function* CustomNewDeleteDebug::CUSTOM_NEW_NO_THROW_FUNC = nullptr;
Function* CustomNewDeleteDebug::CUSTOM_DELETE_FUNC = nullptr;

const map<StringRef, Function**> CustomNewDeleteDebug::OP_TO_CUSTOM = {
  { NEW_OPS[0], &CUSTOM_NEW_FUNC },
  { NEW_OPS[1], &CUSTOM_NEW_FUNC },
  { NEW_NO_THROW_OPS[0], &CUSTOM_NEW_NO_THROW_FUNC },
  { NEW_NO_THROW_OPS[1], &CUSTOM_NEW_NO_THROW_FUNC }
};

GlobalVariable* CustomNewDeleteDebug::UNKNOWN_TYPE = nullptr;
GlobalVariable* CustomNewDeleteDebug::UNKNOWN_FUNC = nullptr;

static RegisterPass<CustomNewDeleteDebug> X("custom new delete debug",
                                            "CustomNewDeleteDebug Pass",
                                            false /* Only looks at CFG */,
                                            false /* Analysis Pass */);

static void registerMyPass(const PassManagerBuilder &,
                           PassManagerBase &PM) {
  PM.add(new CustomNewDeleteDebug());
}
static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerMyPass);
