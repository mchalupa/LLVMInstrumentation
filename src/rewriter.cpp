#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include "rewriter.hpp"
#include "json/json.h"

using namespace std;

static void getFindInstructions(RewriteRule& r, const Json::Value& instruction_rules)
{
    const auto& find_instructions = instruction_rules["findInstructions"];

    for (uint k = 0; k < find_instructions.size(); ++k) {
        InstrumentInstruction instr;
        const auto& instruction = find_instructions[k];
        instr.returnValue = instruction["returnValue"].asString();
        instr.instruction = instruction["instruction"].asString();

        const auto& instruction_operands = instruction["operands"];
        for (uint j = 0; j < instruction_operands.size(); ++j) {
            instr.parameters.push_back(instruction_operands[j].asString());
        }

        instr.getSizeTo = instruction["getSizeTo"].asString();
        instr.stripInboundsOffsets = instruction["stripInboundsOffsets"].asString();
        r.foundInstrs.push_back(std::move(instr));
    }
}

static void getNewInstruction(RewriteRule& r, const Json::Value& instruction_rules)
{
    const auto& new_instruction = instruction_rules["newInstruction"];
    r.newInstr.returnValue = new_instruction["returnValue"].asString();
    r.newInstr.instruction = new_instruction["instruction"].asString();

    const auto& new_instruction_operands = new_instruction["operands"];
    for (uint j = 0; j < new_instruction_operands.size(); ++j) {
        r.newInstr.parameters.push_back(new_instruction_operands[j].asString());
    }

    const auto& instruction_placement = instruction_rules["where"];
    if (instruction_placement == "before") {
        r.where = InstrumentPlacement::BEFORE;
    }
    else if (instruction_placement == "after") {
        r.where = InstrumentPlacement::AFTER;
    }
    else if (instruction_placement == "replace") {
        r.where = InstrumentPlacement::REPLACE;
    }
    else if (instruction_placement == "return") {
        r.where = InstrumentPlacement::RETURN;
    }
    else if (instruction_placement == "entry") {
        r.where = InstrumentPlacement::ENTRY;
    }

    r.inFunction = instruction_rules["in"].asString();

    const auto& instr_condition = instruction_rules["condition"];
    for(uint j = 0; j < instr_condition.size(); ++j){
        r.condition.push_back(instr_condition[j].asString());
    }
}

static RewritePhase getPhase(const Json::Value& current_phase)
{
    RewritePhase phase;

    /// ------------------------------------
    // Get instruction rules.
    /// ------------------------------------
    const auto& instruction_rules = current_phase["instructionRules"];
    for (uint i = 0; i < instruction_rules.size(); ++i) {
        RewriteRule r;
        const auto& current_instruction_rules = instruction_rules[i];

        getFindInstructions(r, current_instruction_rules);
        getNewInstruction(r, current_instruction_rules);

        phase.getConfig().emplace_back(std::move(r));
    }

    /// ------------------------------------
    // Get rule for global variables.
    /// ------------------------------------
    GlobalVarsRule& rw_globals_rule = phase.getGlobalsConfig();

    const auto& global_variables_rule = current_phase["globalVariablesRule"];
    const auto& find_globals = global_variables_rule["findGlobals"];
    rw_globals_rule.globalVar.globalVariable = find_globals["globalVariable"].asString();
    rw_globals_rule.globalVar.getSizeTo = find_globals["getSizeTo"].asString();

    const auto& gv_condition = global_variables_rule["condition"];
    for(uint j = 0; j < gv_condition.size(); ++j){
        rw_globals_rule.condition.push_back(gv_condition[j].asString());
    }

    const auto& gv_new_instruction = global_variables_rule["newInstruction"];
    rw_globals_rule.newInstr.returnValue = gv_new_instruction["returnValue"].asString();
    rw_globals_rule.newInstr.instruction = gv_new_instruction["instruction"].asString();

    const auto& gv_new_instruction_operands = gv_new_instruction["operands"];
    for (uint j = 0; j < gv_new_instruction_operands.size(); ++j) {
        rw_globals_rule.newInstr.parameters.push_back(gv_new_instruction_operands[j].asString());
    }

    rw_globals_rule.inFunction = global_variables_rule["in"].asString();

    return phase;
}

void Rewriter::parseConfig(ifstream &config_file) {
    Json::Value json_rules;
    Json::Reader reader;

    bool parsingSuccessful = reader.parse(config_file, json_rules);
    if (!parsingSuccessful)    {
        cerr  << "Failed to parse configuration\n"
              << reader.getFormattedErrorMessages();
        throw runtime_error("Config parsing failure.");
    }

    // TODO catch exceptions here

    // load paths to analyses
    const auto& analyses = json_rules["analyses"];
    for(uint i = 0; i < analyses.size(); ++i){
        this->analysisPaths.emplace_back(analyses[i].asString());
    }

    const auto& config_phases = json_rules["phases"];
    if (config_phases.empty()) {
        cerr << "No instrumentation phase found in config, aborting\n";
        throw runtime_error("Invalid config.");
    }

    for (uint phase_idx = 0; phase_idx < config_phases.size(); ++phase_idx) {

        // load rewrite rules for instructions
        const auto& current_phase = config_phases[phase_idx];
        phases.push_back(getPhase(current_phase));
    }
}

