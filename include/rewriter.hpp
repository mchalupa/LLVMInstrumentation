#ifndef REWRITER_H
#define REWRITER_H

#include <list>
#include <map>
#include <set>
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
    // set a variable (the first of the pair)
    // to given value (the second of the pair)
    std::pair<std::string, std::string> set_variable;
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

class Environment {
    using MappingT = std::map<const std::string, const std::string>;

    MappingT environment;
    // Set of known variables. It is not identical to keys
    // of the environment, it is the set of variables
    // that appear in configuration files. In the keys
    // of environment mapping are only the set variables.
    std::set<std::string> known_variables;

public:
    void addVariable(const std::string& var) {
        known_variables.insert(var);
    }

    void setVariable(const std::string& var, const std::string& val) {
        known_variables.insert(var);
        environment.emplace(var, val);
    }

    bool isVariable(const std::string& var) const {
        return known_variables.count(var) > 0;
    }

    const std::string& get(const std::string& var) const {
        static std::string none;

        auto it = environment.find(var);
        if (it == environment.end())
            return none;
        return it->second;;
    }
};

// Rewriter
class Rewriter {

    // phases of instrumentation
    std::list<RewritePhase> phases;

    // variables that may be set by phases
    Environment environment;

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

        Environment& getEnvironment() {
            return environment;
        }
};

#endif
