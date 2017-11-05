//
// Created by fedor on 22/09/17.
//

#include "ap.h"
#include "pdrh_config.h"
#include "model.h"
#include "box_factory.h"
#include <capd/capdlib.h>
#include <capd/intervals/lib.h>

pdrh::type ap::model_type;
pair<pdrh::node*, pdrh::node*> ap::time;
map<string, tuple<pdrh::node*, pdrh::node*, pdrh::node*, pdrh::node*>> ap::rv_map;
map<string, string> ap::rv_type_map;
map<string, map<pdrh::node*, pdrh::node*>> ap::dd_map;
map<string, pair<pdrh::node*, pdrh::node*>> ap::var_map;
map<string, pair<pdrh::node*, pdrh::node*>> ap::par_map;
map<string, pdrh::node*> ap::syn_map;
vector<pdrh::mode> ap::modes;
vector<pdrh::state> ap::init;
vector<pdrh::state> ap::goal;
vector<vector<pdrh::mode*>> ap::paths;

map<string, pair<pdrh::node*, pdrh::node*>> ap::distribution::uniform;
map<string, pair<pdrh::node*, pdrh::node*>> ap::distribution::normal;
map<string, pdrh::node*> ap::distribution::exp;
map<string, pair<pdrh::node*, pdrh::node*>> ap::distribution::gamma;

void ap::copy_model()
{
    ap::model_type = pdrh::model_type;
    ap::time = pdrh::time;
    ap::rv_map = pdrh::rv_map;
    ap::rv_type_map = pdrh::rv_type_map;
    ap::dd_map = pdrh::dd_map;
    ap::var_map = pdrh::var_map;
    ap::par_map = pdrh::par_map;
    ap::syn_map = pdrh::syn_map;
    ap::modes = pdrh::modes;
    ap::init = pdrh::init;
    ap::goal = pdrh::goal;
    ap::paths = pdrh::paths;
    ap::distribution::uniform = pdrh::distribution::uniform;
    ap::distribution::normal = pdrh::distribution::normal;
    ap::distribution::exp = pdrh::distribution::exp;
    ap::distribution::gamma = pdrh::distribution::gamma;
}

void ap::modify_model()
{
    // removing all non-time variables
    map<string, pair<pdrh::node*, pdrh::node*>> var_map;
    for(auto it = pdrh::var_map.begin(); it != pdrh::var_map.end(); it++)
    {
        if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
        {
            var_map.insert(make_pair(it->first, it->second));
        }
    }
    pdrh::var_map = var_map;

    // removing all non-time parameters
    map<string, pair<pdrh::node*, pdrh::node*>> par_map;
    for(auto it = pdrh::par_map.begin(); it != pdrh::par_map.end(); it++)
    {
        if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
        {
            par_map.insert(make_pair(it->first, it->second));
        }
    }
    pdrh::par_map = par_map;

    for(int i = 0; i < pdrh::modes.size(); i++)
    {
        // removing non-time variables from the flow map
        map<string, pair<pdrh::node*, pdrh::node*>> flow_map;
        for(auto it = pdrh::modes.at(i).flow_map.begin(); it != pdrh::modes.at(i).flow_map.end(); it++)
        {
            if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
            {
                flow_map.insert(make_pair(it->first, it->second));
            }
        }
        pdrh::modes.at(i).flow_map = flow_map;

        // removing non-time variables from the ode map
        map<string, pdrh::node*> odes;
        for(auto it = pdrh::modes.at(i).odes.begin(); it != pdrh::modes.at(i).odes.end(); it++)
        {
            if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
            {
                odes.insert(make_pair(it->first, it->second));
            }
        }
        pdrh::modes.at(i).odes = odes;

        // removing non-time variables from the resets and the guards
        for(int j = 0; j < pdrh::modes.at(i).jumps.size(); j++)
        {
            // removing non-time variables from the resets
            map<string, pdrh::node*> reset;
            for(auto it = pdrh::modes.at(i).jumps.at(j).reset.begin(); it != pdrh::modes.at(i).jumps.at(j).reset.end(); it++)
            {
                if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
                {
                    reset.insert(make_pair(it->first, it->second));
                }
            }
            pdrh::modes.at(i).jumps.at(j).reset = reset;

            // removing non-time variables from the dd map
            map<string, map<pdrh::node*, pdrh::node*>> reset_dd;
            for(auto it = pdrh::modes.at(i).jumps.at(j).reset_dd.begin(); it != pdrh::modes.at(i).jumps.at(j).reset_dd.end(); it++)
            {
                if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
                {
                    reset_dd.insert(make_pair(it->first, it->second));
                }
            }
            pdrh::modes.at(i).jumps.at(j).reset_dd = reset_dd;

            // removing non-time variables from the rv map
            map<string, tuple<string, pdrh::node*, pdrh::node*, pdrh::node*, pdrh::node*>> reset_rv;
            for(auto it = pdrh::modes.at(i).jumps.at(j).reset_rv.begin(); it != pdrh::modes.at(i).jumps.at(j).reset_rv.end(); it++)
            {
                cout << it->first << endl;
                if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
                {
                    reset_rv.insert(make_pair(it->first, it->second));
                }
            }
            pdrh::modes.at(i).jumps.at(j).reset_rv = reset_rv;

            // removing non-time variables from the nondet map
            map<string, pair<pdrh::node*, pdrh::node*>> reset_nondet;
            for(auto it = pdrh::modes.at(i).jumps.at(j).reset_nondet.begin(); it != pdrh::modes.at(i).jumps.at(j).reset_nondet.end(); it++)
            {
                if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) != global_config.time_var_name.end())
                {
                    reset_nondet.insert(make_pair(it->first, it->second));
                }
            }
            pdrh::modes.at(i).jumps.at(j).reset_nondet = reset_nondet;
        }

    }

    // pdrh::modes.clear();
    // pdrh::init.clear();
    // pdrh::goal.clear();
    // pdrh::paths.clear();
}

void ap::nullify_odes()
{
    for(int i = 0; i < pdrh::modes.size(); i++)
    {
        // removing non-time variables from the ode map
        for(auto it = pdrh::modes.at(i).odes.begin(); it != pdrh::modes.at(i).odes.end(); it++)
        {
            if(std::find(global_config.time_var_name.begin(), global_config.time_var_name.end(), it->first) == global_config.time_var_name.end())
            {
                it->second = new pdrh::node("0");
            }
        }
    }
}

void ap::revert_model()
{
    pdrh::model_type = ap::model_type;
    pdrh::time = ap::time;
    pdrh::rv_map = ap::rv_map;
    pdrh::rv_type_map = ap::rv_type_map;
    pdrh::dd_map = ap::dd_map;
    pdrh::var_map = ap::var_map;
    pdrh::par_map = ap::par_map;
    pdrh::syn_map = ap::syn_map;
    pdrh::modes = ap::modes;
    pdrh::init = ap::init;
    pdrh::goal = ap::goal;
    pdrh::paths = ap::paths;
    pdrh::distribution::uniform = ap::distribution::uniform;
    pdrh::distribution::normal = ap::distribution::normal;
    pdrh::distribution::exp = ap::distribution::exp;
    pdrh::distribution::gamma = ap::distribution::gamma;
}

capd::interval ap::get_sample_rate(pdrh::node* n)
{
    pdrh::node *node_copy = new pdrh::node;
    pdrh::copy_tree(node_copy, n);
    pdrh::node* time_node = new pdrh::node;
    pdrh::get_first_time_node(node_copy, time_node);
    // checking if the time node is not empty
    if(!pdrh::is_node_empty(time_node))
    {
        // checking if the time node signature is <var>=<value>
        if(time_node->value == "=")
        {
            if(time_node->operands.front()->value == "counter")
            {
                return pdrh::node_to_interval(time_node->operands.back());
            }
            if(time_node->operands.back()->value == "counter")
            {
                return pdrh::node_to_interval(time_node->operands.front());
            }
        }
    }
    return capd::interval(0.0);
}


capd::interval ap::get_sample_rate(pdrh::mode* m)
{
    for(pdrh::mode::jump j : m->jumps)
    {
        capd::interval sample_rate = ap::get_sample_rate(j.guard);
        if(sample_rate != capd::interval(0.0))
        {
            return sample_rate;
        }
    }
    return capd::interval(0.0);
}


capd::interval ap::get_meal_time(pdrh::node *n, vector<box> boxes)
{
    pdrh::node *node_copy = new pdrh::node();
    pdrh::copy_tree(node_copy, n);
    pdrh::node* time_node = new pdrh::node;
    pdrh::get_first_time_node(node_copy, time_node);
    // checking if the time node is not empty
    if(!pdrh::is_node_empty(time_node))
    {
        // checking if the time node signature is <var>=<value>
        if(time_node->value == "=")
        {
            if(time_node->operands.front()->value == "tau")
            {
                for(box b : boxes)
                {
                    for(auto it = b.get_map().begin(); it != b.get_map().end(); it++)
                    {
                        if(it->first == time_node->operands.back()->value)
                        {
                            return it->second;
                        }
                    }
                }
                return pdrh::node_to_interval(time_node->operands.back());
            }
            if(time_node->operands.back()->value == "tau")
            {
                for(box b : boxes)
                {
                    for(auto it = b.get_map().begin(); it != b.get_map().end(); it++)
                    {
                        if(it->first == time_node->operands.front()->value)
                        {
                            return it->second;
                        }
                    }
                }
                return pdrh::node_to_interval(time_node->operands.front());
            }
        }
    }
    return capd::interval(0.0);
}


capd::interval ap::get_meal_time(pdrh::mode *m, vector<box> boxes)
{
    for(pdrh::mode::jump j : m->jumps)
    {
        capd::interval meal_time = ap::get_meal_time(j.guard, boxes);
        if(meal_time != capd::interval(0.0))
        {
            return meal_time;
        }
    }
    return capd::interval(0.0);
}


int ap::jumps_per_mode(pdrh::mode *m, vector<box> boxes)
{
    capd::interval sample_rate = ap::get_sample_rate(m);
    for(pdrh::state st : pdrh::goal)
    {
        if(m->id == st.id)
        {
            return ceil((ap::get_meal_time(st.prop, boxes) / sample_rate).rightBound());
        }
    }
    return ceil((ap::get_meal_time(m, boxes) / sample_rate).rightBound());
}


int ap::jumps_per_mode(pdrh::mode *cur_mode, pdrh::mode *prev_mode, vector<box> boxes)
{
    capd::interval left_over;
    // checking if there is previous mode
    if(prev_mode->id > 0)
    {
        capd::interval sample_rate_prev_mode = ap::get_sample_rate(prev_mode);
        capd::interval meal_time_prev_mode = ap::get_meal_time(prev_mode, boxes);
        left_over = capd::interval(fmod(meal_time_prev_mode.leftBound(), sample_rate_prev_mode.leftBound()),
                fmod(meal_time_prev_mode.rightBound(), sample_rate_prev_mode.rightBound()));
    }
    else
    {
        left_over = capd::interval(0);
    }

    capd::interval sample_rate_cur_mode = ap::get_sample_rate(cur_mode);
    for(pdrh::state st : pdrh::goal)
    {
        if(cur_mode->id == st.id)
        {
            return ceil(((ap::get_meal_time(st.prop, boxes) + left_over) / sample_rate_cur_mode).rightBound());
        }
    }
    return ceil(((ap::get_meal_time(cur_mode, boxes) + left_over) / sample_rate_cur_mode).rightBound());
}

bool ap::accept_path(vector<pdrh::mode *> path, vector<box> boxes)
{
    int pos = 0;
    // creating an empty mode
    pdrh::mode *prev_mode = new pdrh::mode;
    prev_mode->id = 0;
    while(pos < path.size())
    {
        pdrh::mode *cur_mode = path.at(pos);
        int num_reps = 1;
        for(int i = pos + 1; i < path.size(); i++)
        {
            if(path.at(i)->id == cur_mode->id)
            {
                num_reps++;
            }
            else
            {
                break;
            }
        }
        int num_jumps = 0;
        if(prev_mode->id == 0)
        {
            num_jumps = ap::jumps_per_mode(cur_mode, boxes);
        }
        else
        {
            num_jumps = ap::jumps_per_mode(cur_mode, prev_mode, boxes);
        }
        //cout << "Num reps: " << num_reps << endl;
        //cout << "Num jumps: " << num_jumps << endl;
        if(num_reps != num_jumps)
        {
            return false;
        }
        pos += num_reps;
        prev_mode = cur_mode;
    }
    return true;
}


box ap::init_to_box()
{
    map<string, capd::interval> b_map;
    vector<capd::interval> b_interval = {   capd::interval("-4.390166457000773e-35","-4.390166457000703e-35"),
                                            capd::interval("4.626459344146126e-08","4.626459344146168e-08"),
                                            capd::interval("0.03351449275362305","0.03351449275362305"),
                                            capd::interval("97.77777777777777","97.77777777777777"),
                                            capd::interval("97.77777777777777","97.77777777777777"),
                                            capd::interval("19.08024296516837","19.08024296516837"),
                                            capd::interval("3.0525","3.0525"),
                                            capd::interval("3.0525","3.0525"),
                                            capd::interval("0","0"),
                                            capd::interval("0","0"),
                                            capd::interval("0","0"),
                                            capd::interval("0","0"),
                                            capd::interval("97.77777777777777","97.77777777777777"),
                                            capd::interval("0","0"),
                                            capd::interval("0.01899154566043506","0.01899154566043506"),
                                            capd::interval("0.03128019323671478","0.03128019323671478"),
                                            capd::interval("0.0268115942028984","0.0268115942028984")};

//    vector<capd::interval> b_interval = {   capd::interval(-4.390166457000773e-35),
//                                            capd::interval(4.626459344146126e-08),
//                                            capd::interval(0.03351449275362305),
//                                            capd::interval(97.77777777777777),
//                                            capd::interval(97.77777777777777),
//                                            capd::interval(19.08024296516837),
//                                            capd::interval(3.0525),
//                                            capd::interval(3.0525),
//                                            capd::interval(0),
//                                            capd::interval(0),
//                                            capd::interval(0),
//                                            capd::interval(0),
//                                            capd::interval(97.77777777777777),
//                                            capd::interval(0),
//                                            capd::interval(0.01899154566043506),
//                                            capd::interval(0.03128019323671478),
//                                            capd::interval(0.0268115942028984)};

    int i = 0;
    for(auto it = pdrh::modes.front().odes.begin(); it != pdrh::modes.front().odes.end(); it++)
    {
        b_map.insert(make_pair(it->first, b_interval.at(i)));
        i++;
    }
    return box(b_map);
}


box ap::solve_odes(map<string, pdrh::node *> odes, box init, capd::interval time, vector<box> boxes)
{
    // creating capd string here
    // declaring parameters
    string par_string = "par:";
    for(box b : boxes)
    {
        map<string, capd::interval> b_map = b.get_map();
        for(auto it = b_map.begin(); it != b_map.end(); it++)
        {
            par_string += it->first + ',';
        }
    }
    par_string.back() = ';';

    // declaring variables
    string var_string = "var:";
    string fun_string = "fun:";
    for(auto it = odes.begin(); it != odes.end(); it++)
    {
        var_string += it->first + ',';
        fun_string += pdrh::node_to_string_infix(it->second) + ',';
    }
    var_string.back() = ';';
    fun_string.back() = ';';

    // creating an ODE solver and setting precision
    capd::IMap vectorField(var_string + fun_string);
    capd::IOdeSolver solver(vectorField, 20);
    solver.setAbsoluteTolerance(1e-12);
    solver.setRelativeTolerance(1e-12);
    capd::ITimeMap timeMap(solver);

    //setting parameter values
    for(box b : boxes)
    {
        map<string, capd::interval> b_map = b.get_map();
        for(auto it = b_map.begin(); it != b_map.end(); it++)
        {
            vectorField.setParameter(it->first, it->second);
        }
    }

    // setting initial condition here
    capd::IVector c(odes.size());
    map<string, capd::interval> init_map = init.get_map();
    int i = 0;
    for(auto it = init_map.begin(); it != init_map.end(); it++)
    {
        c[i] = it->second;
        i++;
    }
    capd::C0HORect2Set s(c);

    // solving the ODE system and creating the result
    capd::IVector result = timeMap(time, s);
    map<string, capd::interval> res_map;
    i = 0;
    for(auto it = odes.begin(); it != odes.end(); it++)
    {
        res_map.insert(make_pair(it->first, result[i]));
        i++;
    }
    return box(res_map);
}


capd::interval time_to_goal(pdrh::mode *m, vector<box> boxes)
{
    for(pdrh::state st : pdrh::goal)
    {
        if(m->id == st.id)
        {
            return ap::get_meal_time(st.prop, boxes);
        }
    }
}


box ap::simulate_path(vector<pdrh::mode *> path, box init, vector<box> boxes)
{
    // reachability depth == 0
    if(path.size() == 1)
    {
        return {solve_odes(path.front()->odes, init, time_to_goal(path.front(), boxes), boxes)};
    }
    // reachability depth > 0
    vector<box> sol_box;
    vector<box> init_box = {init};
    cout << "Init box:" << endl;
    cout << init_box << endl;
    cout << "------" << endl;
//    int dummy;
//    cin >> dummy;
    capd::interval cur_mode_time(0);
    capd::interval prev_mode_time(0);
    int branch_num = 0;
    int window_size = 11;
    for(size_t j = 0; j < path.size() - 1; j = j + window_size)
    {
        for(size_t i = j; (i < j + window_size) && (i < path.size() - 1); i++)
        {
            pdrh::mode *cur_mode = path.at(i);
            pdrh::mode *next_mode = path.at(i + 1);
            // solving the odes here
            if(cur_mode->id == next_mode->id)
            {
                capd::interval time = ap::get_sample_rate(cur_mode) - prev_mode_time;
                for(size_t k = 0; k < init_box.size(); k++)
                {
                    sol_box.push_back(solve_odes(cur_mode->odes, init_box.at(k), time, boxes));
                }
                cur_mode_time += time;
                prev_mode_time = capd::interval(0);
            }
            else
            {
                capd::interval time = ap::get_meal_time(cur_mode, boxes) - cur_mode_time;
                for(size_t k = 0; k < init_box.size(); k++)
                {
                    sol_box.push_back(solve_odes(cur_mode->odes, init_box.at(k), time, boxes));
                }
                cur_mode_time = capd::interval(0);
                prev_mode_time = time;
            }
            cout << "Mode = " << cur_mode->id << ". Depth = " << i << endl;
            cout << "Solution boxes hull:" << endl;
            cout << box_factory::box_hull(sol_box) << endl;
            vector<box> part_sol_box;
            for(box b : sol_box)
            {
//            cout << j << ")" << endl;
//            cout << b << endl;
                vector<box> part_boxes = box_factory::bisect(b, {"Q1"});
                part_sol_box.insert(part_sol_box.end(), part_boxes.begin(), part_boxes.end());
            }
//        cout << "There are " << part_sol_box.size() << " boxes after partitioning" << endl;
//        cout << "Boxes after partitioning" << endl;
//        for(box b : part_sol_box)
//        {
//            cout << b << endl;
//            cout << "----------" << endl;
//        }
            //sol_box = part_sol_box.back();

//        int dummy;
//        cin >> dummy;

            // resetting the initial state for the next mode
            init_box.clear();
            map<string, pdrh::node*> reset_map = cur_mode->get_jump(next_mode->id).reset;
            for(box b : part_sol_box)
            {
                map<string, capd::interval> init_map;
                for (auto it = reset_map.begin(); it != reset_map.end(); it++)
                {
                    init_map.insert(make_pair(it->first, pdrh::node_to_interval(it->second, b)));
                }
                init_box.push_back(box(init_map));
            }
            // clear solution boxes
            sol_box.clear();
            part_sol_box.clear();
        }
        init_box = { box_factory::box_hull(init_box) };
    }

    // the last (goal flow) flow is here
    for(size_t k = 0; k < init_box.size(); k++)
    {
        // cout << "Going through box " << b << endl;
        sol_box.push_back(solve_odes(path.back()->odes, init_box.at(k), time_to_goal(path.back(), boxes) - cur_mode_time, boxes));
    }
    return box_factory::box_hull(sol_box);
}



vector<vector<pdrh::mode*>> ap::get_all_paths(vector<box> boxes)
{
    // getting list of shortest paths
    vector<vector<pdrh::mode*>> paths;
    vector<vector<pdrh::mode*>> res;
    for(pdrh::state i : pdrh::init)
    {
        for(pdrh::state g : pdrh::goal)
        {
            vector<pdrh::mode*> path = pdrh::get_shortest_path(pdrh::get_mode(i.id), pdrh::get_mode(g.id));
            if(path.size() > 0)
            {
                if(path.size() == 1)
                {
                    res.push_back(path);
                }
                else
                {
                    paths.push_back(path);
                }
            }
        }
    }
    // inserting self-loops in each path
    for(vector<pdrh::mode*> path : paths)
    {
        vector<pdrh::mode*> new_path;
        pdrh::mode* prev_mode = new pdrh::mode;
        prev_mode->id = 0;
        for(size_t i = 0; i < path.size(); i++)
        {
            pdrh::mode* cur_mode = path[i];
            int num_jumps;
            if(i < path.size() - 1)
            {
                num_jumps = ap::jumps_per_mode(cur_mode, prev_mode, boxes);
            }
            else
            {
                num_jumps = ap::jumps_per_mode(cur_mode, boxes);
            }
            for(int j = 0; j < num_jumps; j++)
            {
                new_path.push_back(cur_mode);
            }
            prev_mode = cur_mode;
        }
        new_path.push_back(path[path.size()-1]);
        res.push_back(new_path);
    }
    return res;
}

















