#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include "rewriter.hpp"
#include "json/json.h"

using namespace std;

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
    for(uint i = 0; i < json_rules["analyses"].size(); ++i){
        this->analysisPaths.push_back(json_rules["analyses"][i].asString());
    }

    const auto& config_phases = json_rules["phases"];
    if (config_phases.empty()) {
        cerr << "No instrumentation phase found in config, aborting\n";
        throw runtime_error("Invalid config.");
    }

    for (uint phase_idx = 0; phase_idx < config_phases.size(); ++phase_idx) {
        RewritePhase phase;
        
        // load rewrite rules for instructions
        const auto& current_phase = config_phases[phase_idx];
        const auto& instruction_rules = current_phase["instructionRules"];
        for (uint i = 0; i < instruction_rules.size(); ++i) {
            RewriteRule r;
        
            // TODO make function from this
            // Get findInstructions
            const auto& find_instructions = instruction_rules[i]["findInstructions"];
            for (uint k = 0; k < find_instructions.size(); ++k) {
                InstrumentInstruction instr;
                instr.returnValue = find_instructions[k]["returnValue"].asString();
                instr.instruction = find_instructions[k]["instruction"].asString();
                for (uint j = 0; j < find_instructions[k]["operands"].size(); ++j) {
                    instr.parameters.push_back(find_instructions[k]["operands"][j].asString());
                }
                instr.getSizeTo = find_instructions[k]["getSizeTo"].asString();
                instr.stripInboundsOffsets = find_instructions[k]["stripInboundsOffsets"].asString();
                r.foundInstrs.push_back(instr);
            }
        
            // Get newInstruction
            r.newInstr.returnValue = instruction_rules[i]["newInstruction"]["returnValue"].asString();
            r.newInstr.instruction = instruction_rules[i]["newInstruction"]["instruction"].asString();
            for (uint j = 0; j < instruction_rules[i]["newInstruction"]["operands"].size(); ++j) {
                r.newInstr.parameters.push_back(instruction_rules[i]["newInstruction"]["operands"][j].asString());
            }
        
            const auto& instruction_placement = instruction_rules[i]["where"];
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
        
            r.inFunction = instruction_rules[i]["in"].asString();
        
            for(uint j = 0; j < instruction_rules[i]["condition"].size(); ++j){
                r.condition.push_back(instruction_rules[i]["condition"][j].asString());
            }
        
            phase.getConfig().push_back(r);
        }
        
        GlobalVarsRule& rw_globals_rule = phase.getGlobalsConfig();
        
        // Get rule for global variables
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
        
        phases.push_back(phase);
    }
}

