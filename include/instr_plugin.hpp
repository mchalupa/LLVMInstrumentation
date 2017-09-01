#ifndef INSTR_PLUGIN_H
#define INSTR_PLUGIN_H

#include <string>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Operator.h>

class InstrPlugin
{
    protected:
      // optional name for the plugin
      std::string name;

	public:
      InstrPlugin() = default;
      InstrPlugin(const std::string& nm) : name(nm) {}

      const std::string& getName() { return name; }

      // the default behaviour is returning true, since we have
      // no information about the value, so it may be anything
      virtual bool isNull(llvm::Value*) {
        return true;
      }

      // XXX: we should probably rename it to isValidPointerRange
      virtual bool isValidPointer(llvm::Value*, llvm::Value *) {
        return true;
      }

      virtual bool isEqual(llvm::Value*, llvm::Value*) {
        return true;
      }

      virtual bool isNotEqual(llvm::Value*, llvm::Value*) {
        return true;
      }

      virtual bool greaterThan(llvm::Value*, llvm::Value*) {
        return true;
      }

      virtual bool lessThan(llvm::Value*, llvm::Value*) {
        return true;
      }

      virtual bool lessOrEqual(llvm::Value*, llvm::Value*) {
        return true;
      }

      virtual bool greaterOrEqual(llvm::Value*, llvm::Value*) {
        return true;
      }
	  
      virtual bool isConstant(llvm::Value* a) {
        return llvm::isa<llvm::Constant>(a);
	  }

      virtual bool canOverflow(llvm::Value *a) {
        if (llvm::OverflowingBinaryOperator *O
            = llvm::dyn_cast<llvm::OverflowingBinaryOperator>(a)) {
          return !O->hasNoSignedWrap();
        }

        return false;
      }

      // add virtual destructor, so that child classes will
      // call their destructor
      virtual ~InstrPlugin() {}
};

#endif
