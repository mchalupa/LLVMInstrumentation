#ifndef CALLGRAPH_H
#define CALLGRAPH_H

#include <map>
#include <set>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include "llvm/analysis/PointsTo/PointsTo.h"
#include "analysis/PointsTo/PointsToFlowInsensitive.h"

class CGNode {
	public:
		CGNode(const llvm::Function* callerF, int id);
		bool containsCall(std::vector<CGNode> nodeMapping, const llvm::Function* call);
		const llvm::Function* getCaller() const;
		int getId() const;
		std::vector<int> calls;

	private:
		int id;
		const llvm::Function* caller;
};

class CallGraph {
	public:
		std::vector<CGNode> nodes;
		CallGraph(llvm::Module &m, std::unique_ptr<dg::LLVMPointerAnalysis> &pta) : M(m), PTA(pta) {}

        void compute();
		bool containsCall(const llvm::Function* caller, const llvm::Function* callee);
		bool containsDirectCall(const llvm::Function* caller, const llvm::Function* callee);
		bool isRecursive(const llvm::Function* function);

        std::set<const llvm::Function *> getReachableFunctions(const llvm::Function *start) const;

	private:
		int lastId;
		void handleCallInst(std::unique_ptr<dg::LLVMPointerAnalysis> &PTA, const llvm::Function *F, const llvm::CallInst *CI);
		bool BFS(const CGNode startNode, std::vector<bool> &visited);
		int findNode(const llvm::Function* function) const;

        llvm::Module& M;
        std::unique_ptr<dg::LLVMPointerAnalysis>& PTA;
};

#endif
