//
// Created by kny on 27/02/18.
//

#include <sstream>
#include <logging/easylogging++.h>
#include <util/pdrh.h>

#include "MatlabEngine.hpp"
#include "MatlabDataArray.hpp"
#include "translator.h"
#include "pdrh_config.h"
#include "translator_util.h"


using namespace std;
using namespace matlab::engine;
using namespace matlab::data;

const string translator::slStateBase = "mode";
const string translator::subSysHandlerBase = "subSysHandler";

//TODO: read number of jump counts from main (it's command line option)
void print_map(map<string, pair<pdrh::node*, pdrh::node*>>& map1){
    for (auto& t : map1)
        std::cout << t.first << " "
                  << t.second.first->value << " "
                  << t.second.second->value << "\n";
}

translator::Translator::Translator() {
    cout<< "Starting MATLAB engine" << endl;

    this->engine = matlab::engine::startMATLAB();

    cout<< "MATLAB engine started" << endl;

    vector<string> tokenised_filepath = split(global_config.model_filename, '/');
    vector<string> filename = split(tokenised_filepath.at(tokenised_filepath.size() - 1), '.');
    cout<<global_config.model_filename<<endl;
    this->modelName = "prostate_cancer";
    this->systemHandlerName = "h";
    this->parentChart = "c";
}

translator::Translator::~Translator() {
    this->engine.reset();
}

void translator::Translator::set_block_param(const string subSysHandler, const string blkName,
                                             const string parameter, const string value){
    ostringstream command;
    command << "set_param(fullfile(" + subSysHandler +".Path, " << subSysHandler << ".Name, '" << blkName + "'), '"
            << parameter << "', '" << value << "');";
    this->engine->eval(convertUTF8StringToUTF16String(command.str()));
}

string translator::Translator::addBlock(string subSysHandler, string srcPath, string blkName) {
    ostringstream add_block_command;
    add_block_command << systemHandlerName << " = add_block('" << srcPath << "', fullfile(" << subSysHandler
                      << ".Path, " << subSysHandler << ".Name, '" << blkName + "'), 'MakeNameUnique', 'On')";

    this->engine->eval(convertUTF8StringToUTF16String(add_block_command.str()));

    engine->eval(convertUTF8StringToUTF16String("block_name = get_param(h, 'Name')"));
    matlab::data::CharArray blockName = engine->getVariable(convertUTF8StringToUTF16String("block_name"));
    return blockName.toAscii();
}

string translator::Translator::addBlock(string subSysHandler, string srcPath, string blkName,
                                      translator::block_connection &connect_to) {
    string added_block_name = addBlock(subSysHandler, srcPath, blkName);
    ostringstream add_line_command;
    add_line_command << "add_line(fullfile(" << subSysHandler << ".Path, " << subSysHandler << ".Name), '"
                     << added_block_name << "/1', '" << connect_to.block_name << "/" << connect_to.port_id
                     << "', 'autorouting', 'smart')";

    this->engine->eval(convertUTF8StringToUTF16String(add_line_command.str()));

    return added_block_name;
}

void translator::Translator::connect_blocks(string subSysHandler, translator::block_connection out_block,
                                            translator::block_connection in_block) {
    ostringstream add_line_command;
    add_line_command << "add_line(fullfile(" << subSysHandler << ".Path, " << subSysHandler << ".Name), '"
                     << out_block.block_name << "/" << out_block.port_id << "', '"
                     << in_block.block_name << "/" << in_block.port_id
                     << "', 'autorouting', 'smart')";
    this->engine->eval(convertUTF8StringToUTF16String(add_line_command.str()));
}

void translator::Translator::set_system_time_interval(const string& subsys, double start_time, double end_time){
    ostringstream strs;
    strs << "set_param(";
    strs <<  subsys << ", 'StartTime', '" << start_time << "')";
    this->engine->eval(convertUTF8StringToUTF16String(strs.str()));
    strs.clear();
    strs << "set_param(";
    strs << subsys << ", 'EndTime', '" << end_time << "')";
    this->engine->eval(convertUTF8StringToUTF16String(strs.str()));
}

void translator::Translator::generate_init_var_blocks(const pdrh::mode &m){

    for(auto it = m.odes.cbegin(); it != m.odes.cend(); it++){
        string init_value = translator::get_initial_value(it->first);
        cout<< "Initialising blocks for var " << it->first;
        if (strcmp(it->second->value.c_str(), "0") == 0){
            //TODO: Change constant block to integrator block,
            this->addBlock(this->currentSubSystemHandler, "simulink/Continuous/Integrator", it->first);
            this->set_block_param(this->currentSubSystemHandler, it->first, "InitialCondition", init_value);
            this->set_block_param(this->currentSubSystemHandler, it->first, "ContinuousStateAttributes", "''" + it->first + "''");
        } else {
            this->addBlock(this->currentSubSystemHandler, "simulink/Continuous/Integrator", it->first);
            this->set_block_param(this->currentSubSystemHandler, it->first, "InitialCondition", init_value);
            this->set_block_param(this->currentSubSystemHandler, it->first, "ContinuousStateAttributes", "''" + it->first + "''");
        }
    }
}

void translator::Translator::translate_ode_expression(pdrh::node *expr, block_connection parent_block) {
//    cout<< "ODE translation: value: " << expr->value << " and " << expr->operands.size() << " operands." << endl;
    if(expr->operands.size() == 0)
    {
        // the node is a reference to a variable
        if(pdrh::var_exists(expr->value))
        {
            cout<< "Connect " << expr->value << " to already defined variable: " << parent_block.block_name
                << " " << parent_block.port_id <<endl;
            connect_blocks(this->currentSubSystemHandler, block_connection(expr->value, 1), parent_block);
        }
        // the node is a reference to a constant number, assuming 0.0 (zero) is not used in the
        // ODE expression
        else if(strtod(expr->value.c_str(), nullptr) != 0.0)
        {
            string blkName = "Const_" + expr->value;
            addBlock(this->currentSubSystemHandler, "simulink/Commonly Used Blocks/Constant", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, blkName, "Value", expr->value);
        }

    }
    else if(expr->operands.size() > 2)
    {
        CLOG(ERROR, "model") << "The number of operands can't be greater than 2";
        exit(EXIT_FAILURE);
    }
    else
    {
        string blkNameSuffix;
        if (expr->operands.size() == 1){
            blkNameSuffix = expr->operands.at(0)->value.compare("/") != 0 ? expr->operands.at(0)->value + "__":"div__";
        } else {
            blkNameSuffix = expr->operands.at(0)->value.compare("/") != 0 ? expr->operands.at(0)->value:"div";
            blkNameSuffix = blkNameSuffix + "_" +
                            (expr->operands.at(1)->value.compare("/") != 0 ? expr->operands.at(1)->value + "__":"div__");
        }

        if(strcmp(expr->value.c_str(), "+") == 0)
        {
            string blkName = "Add_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Add", blkName, parent_block);

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
            translate_ode_expression(expr->operands.at(1), block_connection(simulink_name, 2));
        }
        else if(strcmp(expr->value.c_str(), "-") == 0)
        {
            if(expr->operands.size() == 1)
            {
                string blkName = "UnaryMinus_" + blkNameSuffix;
                string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Unary Minus", blkName, parent_block);

                translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
            }
            else if(expr->operands.size() == 2)
            {
                string blkName = "Subtract_" + blkNameSuffix;
                string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Subtract", blkName, parent_block);

                translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
                translate_ode_expression(expr->operands.at(1), block_connection(simulink_name, 2));
            }
        }
        else if(strcmp(expr->value.c_str(), "*") == 0)
        {
            string blkName = "Product_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Product", blkName, parent_block);

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
            translate_ode_expression(expr->operands.at(1), block_connection(simulink_name, 2));
        }
        else if(strcmp(expr->value.c_str(), "/") == 0)
        {
            string blkName = "Divide_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Divide", blkName, parent_block);

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
            translate_ode_expression(expr->operands.at(1), block_connection(simulink_name, 2));
        }
        else if(strcmp(expr->value.c_str(), "^") == 0)
        {
            string blkName = "Power_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Math Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "pow");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
            translate_ode_expression(expr->operands.at(1), block_connection(simulink_name, 2));
        }
        else if(strcmp(expr->value.c_str(), "sqrt") == 0)
        {
            string blkName = "Sqrt_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Sqrt", blkName, parent_block);

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "abs") == 0)
        {
            string blkName = "Abs_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Abs", blkName, parent_block);

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "exp") == 0)
        {
            string blkName = "Exp_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Math Function", blkName, parent_block);

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "log") == 0)
        {
            string blkName = "Log_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Math Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "log");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));

        }
        else if(strcmp(expr->value.c_str(), "sin") == 0)
        {
            string blkName = "Sin_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Trigonometric Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "sin");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "cos") == 0)
        {
            string blkName = "Cos_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Trigonometric Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "cos");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "tan") == 0)
        {
            string blkName = "Tan_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Trigonometric Function",
                                            blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "tan");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "asin") == 0)
        {
            string blkName = "Asin_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Trigonometric Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "asin");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "acos") == 0)
        {
            string blkName = "Acos_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Trigonometric Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "acos");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else if(strcmp(expr->value.c_str(), "atan") == 0)
        {
            string blkName = "Atan_" + blkNameSuffix;
            string simulink_name = addBlock(this->currentSubSystemHandler, "simulink/Math Operations/Trigonometric Function", blkName, parent_block);
            set_block_param(this->currentSubSystemHandler, simulink_name, "Operator", "atan");

            translate_ode_expression(expr->operands.at(0), block_connection(simulink_name, 1));
        }
        else
        {
            CLOG(ERROR, "model") << "Unknown function \"" << expr->value << "\"";
            exit(EXIT_FAILURE);
        }
    }
}

// translate a node expression to a stateflow jump condition
string translator::Translator::translate_jump_guard(pdrh::node *guard, int mode_id)
{
    stringstream s, value;
    if (pdrh::var_map.count(guard->value)){
        value << slStateBase << mode_id << "." << guard->value;
    } else if (guard->value == "="){
        value << "==";
    } else if (guard->value == "and"){
        value << " & ";
    }
    else {
        value << guard->value;
    }
    // checking whether it is an operation node
    if(guard->operands.size() > 1)

    {
        s << "(";
        for(int i = 0; i < guard->operands.size() - 1; i++)
        {
            s << translate_jump_guard(guard->operands.at(i), mode_id);
            s << value.str();

        }
        s << translate_jump_guard(guard->operands.at(guard->operands.size() - 1), mode_id) << ")";
    }
    else if(guard->operands.size() == 1)
    {
        if(value.str() ==  "-")
        {
            s << "(" << value.str() << translate_jump_guard(guard->operands.front(), mode_id) << ")";
        }
        else
        {
            s << value.str() << "(" << translate_jump_guard(guard->operands.front(), mode_id) << ")";
        }
    }
    else
    {
        s << value.str();
    }
    return s.str();
}

string translator::Translator::add_state_transition(pdrh::mode& mode){
    stringstream slSourceState;
    slSourceState << slStateBase << mode.id;
    for (pdrh::mode::jump jump : mode.jumps){
        stringstream slDestState;
        slDestState << slStateBase << jump.next_id;
        /**
         * transition_new = Stateflow.Transition(c);
           transition_new.Source = ss1;
           transition_new.Destination = ss2;
         */
        this->engine->eval(convertUTF8StringToUTF16String("new_transition = Stateflow.Transition(" + parentChart + ")"));
        this->engine->eval(convertUTF8StringToUTF16String("new_transition.Source = " + slSourceState.str()));
        this->engine->eval(convertUTF8StringToUTF16String("new_transition.Destination = " + slDestState.str()));

        stringstream transition_label;
        transition_label << "'[" << translate_jump_guard(jump.guard, mode.id)
                         << "]{" << translate_reset_condition(jump, mode.id) << "}'";
        this->engine->eval(convertUTF8StringToUTF16String("new_transition.LabelString = " + transition_label.str()));

        cout<< "Guard: " << translate_jump_guard(jump.guard, mode.id)<<endl;
        cout<< "Resets: " << translate_reset_condition(jump, mode.id)<<endl;
        return "new_transition";
    }
};


void translator::Translator::translate_model(){
    matlab::data::ArrayFactory factory;

    //TODO: bound variables in Simulink: pick initial value in range
    //TODO: non-determinism
    /**
     * Setup the system environment
     */
    this->engine->eval(convertUTF8StringToUTF16String("mdlName = '" + modelName + "'"));
    this->engine->eval(convertUTF8StringToUTF16String(systemHandlerName + " = new_system('" + modelName + "');"));
//    engine->eval(convertUTF8StringToUTF16String(buffer));
    this->engine->eval(convertUTF8StringToUTF16String("open_system(" + systemHandlerName + ");"));
    this->engine->eval(convertUTF8StringToUTF16String("load_system('sflib');"));
    this->engine->eval(convertUTF8StringToUTF16String("add_block('sflib/Chart', ['" + modelName + "' '/Chart']);"));
    this->engine->eval(convertUTF8StringToUTF16String("rt = sfroot;"));
    this->engine->eval(convertUTF8StringToUTF16String("m = rt.find('-isa', 'Stateflow.Machine', 'name', '" + modelName + "');"));
    this->engine->eval(convertUTF8StringToUTF16String(parentChart + " = m.find('-isa', 'Stateflow.Chart');"));
    this->engine->eval(convertUTF8StringToUTF16String(parentChart + ".ChartUpdate = 'CONTINUOUS';"));

    /**
     * Build states and the corresponding ODEs for each state
     */
    for (pdrh::mode m : pdrh::modes){
        ostringstream slState, subSysHandler;
        slState << translator::slStateBase << m.id;
        subSysHandler << translator::subSysHandlerBase << m.id;
        currentSubSystemHandler = subSysHandler.str();
        this->engine->eval(convertUTF8StringToUTF16String(slState.str() + " = Stateflow.SimulinkBasedState(c);"));
        this->engine->eval(convertUTF8StringToUTF16String(slState.str() + ".Name = '" + slState.str() + "';"));
        this->engine->eval(convertUTF8StringToUTF16String(subSysHandler.str() + " = " + slState.str() + ".getDialogProxy;"));

        for(pdrh::state s : pdrh::init){
            if (m.id == s.id){
                add_default_transition(s.id);
            }
        }
        this->generate_init_var_blocks(m);

        for(auto it = m.odes.cbegin(); it != m.odes.cend(); it++){
            this->translate_ode_expression(it->second, block_connection(it->first, 1));
//            engine->eval(convertUTF8StringToUTF16String("Simulink.BlockDiagram.arrangeSystem(" + subSysHandler.str() + ")"));
            cout<<"Completed translating equation d[\"" << it->first << "\"]/dt"<<endl;
        }
    }

    /**
     * Connect all states and set all guards/reset conditions
     */
    for (pdrh::mode m : pdrh::modes){
        add_state_transition(m);
    }
    this->engine->eval(convertUTF8StringToUTF16String("save_system(mdlName);"));
    cout<< "Completed translating model" << endl;
}

string translator::Translator::translate_reset_expression(pdrh::node* reset_expr, int mode_id){
    stringstream s, value;
    if (pdrh::var_exists(reset_expr->value)){
        value << slStateBase << mode_id << "." << reset_expr->value;
    }
    else {
        value << reset_expr->value;
    }
    // checking whether it is an operation reset_expr
    if(reset_expr->operands.size() > 1)

    {
        s << "(";
        for(int i = 0; i < reset_expr->operands.size() - 1; i++)
        {
            s << translate_reset_expression(reset_expr->operands.at(i), mode_id);
            s << value.str();

        }
        s << translate_reset_expression(reset_expr->operands.at(reset_expr->operands.size() - 1), mode_id) << ")";
    }
    else if(reset_expr->operands.size() == 1)
    {
        if(value.str() ==  "-")
        {
            s << "(" << value.str() << translate_reset_expression(reset_expr->operands.front(), mode_id) << ")";
        }
        else
        {
            s << value.str() << "(" << translate_reset_expression(reset_expr->operands.front(), mode_id) << ")";
        }
    }
    else
    {
        s << value.str();
    }
    return s.str();
};


string translator::Translator::translate_reset_condition(pdrh::mode::jump &jump, int source_mode_id) {
    stringstream reset_condition;
    string targetState = slStateBase + to_string(jump.next_id);
    for(auto it = jump.reset.cbegin(); it != jump.reset.cend(); it++){
        reset_condition << targetState << "." << it->first << "="
                        << translate_reset_expression(it->second, source_mode_id) << ";";
    }
        return reset_condition.str();
}

void translator::Translator::add_default_transition(int start_node) {
    stringstream defaultModeName;
    defaultModeName << slStateBase << start_node;
    this->engine->eval(convertUTF8StringToUTF16String("new_transition = Stateflow.Transition(" + parentChart + ");"));
    this->engine->eval(convertUTF8StringToUTF16String("new_transition.Destination = " + defaultModeName.str() + ";"));
    this->engine->eval(convertUTF8StringToUTF16String("new_transition.DestinationOClock = 0;"));
}


void addBlock(const unique_ptr<MATLABEngine>& engine, string systemHandler, string subSysHandler, string srcPath, string blkName) {
    string commandString = systemHandler +  " = add_block('" + srcPath + "', fullfile(" + subSysHandler +".Path, " +
                           subSysHandler + ".Name, '" + blkName + "'))";
    engine->eval(convertUTF8StringToUTF16String(commandString));
}

void addBlock(const unique_ptr<MATLABEngine>& engine, string systemHandler, string subSysHandler,
              string srcPath, string blkName, translator::block_connection previous_block) {
    ostringstream add_block_command;
    add_block_command << systemHandler << " = add_block('" << srcPath << "', fullfile(" << subSysHandler
                      << ".Path, " << subSysHandler << ".Name, '" << blkName + "'))";
    engine->eval(convertUTF8StringToUTF16String(add_block_command.str()));
//    string add_line_command = "add_line(fullfile(" + subSysHandler +".Path, " +
//    subSysHandler + ".Name), '" + blkName + "/1', '" + previous_block.block_name + "/'";
    ostringstream add_line_command;
    add_line_command << "add_line(fullfile(" << subSysHandler << ".Path, " << subSysHandler << ".Name), '"
                     << blkName << "/1', '" << previous_block.block_name << "/" << previous_block.port_id
                     << "', 'autorouting', 'on')";
    engine->eval(convertUTF8StringToUTF16String(add_line_command.str()));
}

int model_creation_test(){
    // Start the Matlab Engine
    unique_ptr<MATLABEngine> engine = startMATLAB();

    matlab::data::ArrayFactory factory;

    string modelName = "testModel";
    string handlerName = "h";

    //TODO: bound variables in Simulink: pick initial value in range
    //TODO: non-determinism
    engine->eval(convertUTF8StringToUTF16String("h = new_system('testModel');"));
//    engine->eval(convertUTF8StringToUTF16String(buffer));
    engine->eval(convertUTF8StringToUTF16String("open_system(h);"));
    engine->eval(convertUTF8StringToUTF16String("load_system('sflib');"));
    engine->eval(convertUTF8StringToUTF16String("add_block('sflib/Chart', ['testModel' '/Chart']);"));
    engine->eval(convertUTF8StringToUTF16String("rt = sfroot;"));
    engine->eval(convertUTF8StringToUTF16String("m = rt.find('-isa', 'Stateflow.Machine', 'name', 'testModel');"));
    engine->eval(convertUTF8StringToUTF16String("c = m.find('-isa', 'Stateflow.Chart');"));

    engine->eval(convertUTF8StringToUTF16String("ss1 = Stateflow.SimulinkBasedState(c);"));
    engine->eval(convertUTF8StringToUTF16String("ss1.Name = 'Gain1';"));
    engine->eval(convertUTF8StringToUTF16String("subsysH = ss1.getDialogProxy;"));

    //TODO: enum or case-switch
    addBlock(engine, "h", "subsysH", "built-in/inport", "in");
    addBlock(engine, "h", "subsysH", "built-in/outport", "out");
    addBlock(engine, "h", "subsysH", "built-in/gain", "K", translator::block_connection("out", 1));

//    set_system_time_interval(engine, "subsysH.Name", 0, 2000.0);

//    engine->eval(convertUTF8StringToUTF16String("Simulink.BlockDiagram.arrangeSystem(subsysH)"));

    engine->eval(convertUTF8StringToUTF16String("save_system('testModel')"));

    matlab::data::Array subsysH = engine->getVariable(convertUTF8StringToUTF16String("subsysH"));
    Array systemHandler = engine->getVariable(convertUTF8StringToUTF16String("h"));

    ArrayType subsysArrType = subsysH.getType();

    matlab::data::Array ss1Handler = engine->getVariable(convertUTF8StringToUTF16String("ss1"));
    matlab::data::CharArray ss1Name = engine->getProperty(ss1Handler, convertUTF8StringToUTF16String("Name"));

    cout<<"Stateflow state name: " + ss1Name.toAscii()<<endl;


    cout<<"Added block"<<endl;

//    engine->eval(convertUTF8StringToUTF16String("figureHandle = figure;"));
//    cout<<"Created system handler"<<endl;
//
//    matlab::data::Array systemHandler = engine->
//            getVariable(convertUTF8StringToUTF16String("figureHandle"));
//
//    matlab::data::CharArray units = engine->
//            getProperty(systemHandler, convertUTF8StringToUTF16String("Units"));
//
//    // Display property value
//    cout << "Units property: " << units.toAscii() << std::endl;


    return 0;
}

// TODO: Find global maximum time bound (in mode or global) multiply by max number of jumps (in pfrh config, reach_depth_max
// , set sample_time to that number + 1  for all generators OR LARGEST NUMBER
string translator::get_initial_value(string variable_name){
    string lower_bound, upper_bound;
    for(pdrh::state s : pdrh::init){
        for(unsigned int i = 0; i < s.prop->operands.size(); i++){
            pdrh::node* node = s.prop->operands.at(i);
            if (node->operands.at(0)->value == variable_name){
                if (node->value == "="){
                    return node->operands.at(1)->value;
                } else {
                    if (node->value == ">="){
                        lower_bound = node->operands.at(1)->value;
                    } else if (node->value == "<="){
                        upper_bound = node->operands.at(1)->value;
                    }
                }
            }
        }
    } return lower_bound + "+(" + upper_bound + "-" + lower_bound + ")*rand(1,1)";
}

void translator::parse_tree(){
    cout<<pdrh::model_to_string();
    print_map(pdrh::var_map);
    print_map(pdrh::par_map);
//    model_creation_test();

    translator::Translator* translator1 = new Translator();
    translator1->translate_model();

    cout<<"Test complete"<<endl;
//    cout<<translator::get_initial_value("y");
    delete translator1;
//    test_engine_call();
}

