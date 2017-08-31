#ifndef REWRITER_H
#define REWRITER_H

#include <list>
#include <string>
#include <iostream>
#include <fstream>


// Configuration
enum class InstrumentPlacement {
	BEFORE,
	AFTER,
	REPLACE,
	RETURN,
	ENTRY
};

class InstrumentInstruction {
 public:
	std::string returnValue;
	std::string instruction;
	std::list<std::string> parameters;
	std::string getSizeTo;
	std::string stripInboundsOffsets;
};

class InstrumentGlobalVar {
 public:
	std::string globalVariable;
	std::string getSizeTo;
};

class GlobalVarsRule {
 public:
	InstrumentGlobalVar globalVar;
	InstrumentInstruction newInstr;
	std::string inFunction;
	std::list<std::string> condition;
};

typedef std::list<InstrumentInstruction> InstrumentSequence;

class RewriteRule {
 public:
	InstrumentSequence foundInstrs;
	InstrumentInstruction newInstr;
	InstrumentPlacement where;
	std::string inFunction;
	std::list<std::string> condition;
};


typedef std::list<RewriteRule> RewriterConfig;

// One phase in instrumenting the module
// -- bears information about what instructions should be
// looked for and how to rewrite them
class RewritePhase {
	RewriterConfig config;
	GlobalVarsRule globalVarsRule;

    // forbid copying this object
    RewritePhase(const RewritePhase&) = delete;

	public:
        RewritePhase() = default;
        RewritePhase(RewritePhase&&) = default;

		const RewriterConfig& getConfig() const { return config; }
		const GlobalVarsRule& getGlobalsConfig() const { return globalVarsRule; }
		RewriterConfig& getConfig() { return config; }
		GlobalVarsRule& getGlobalsConfig() { return globalVarsRule; }
};

// Rewriter
class Rewriter {
    // phases of instrumentation
    std::list<RewritePhase> phases;

    // paths to auxiliary analyses
	std::list<std::string> analysisPaths;

    // forbid copying this object
    Rewriter(const Rewriter&) = delete;

    public:
        Rewriter() = default;

        // parse a configuration file with rules in json
	    void parseConfig(std::ifstream &config_file);

        const std::list<std::string>& getAnalysisPaths() const {
            return analysisPaths;
        }

        const std::list<RewritePhase>& getPhases() const {
            return phases;
        }
};

#endif
