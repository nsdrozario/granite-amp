#include <state.hpp>
#include <sstream>
#include <Nodes.hpp>
#include <fstream>
#include <iostream>
 
std::string adjlist_to_lua() {
    std::cout << "converting adjlist" << std::endl;
    sol::state l;
    l.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
    l.script_file("scripts/io_module.lua");
    sol::table out_adjlist;
    sol::table out_adjlist_inward;
    sol::function serialize_table = l["table_to_str"];
    for (auto k : adjlist) {
        out_adjlist[k.first] = k.second;
    }
    for (auto k : adjlist_inward) {
        out_adjlist_inward[k.first] = k.second;
    }
    std::string str_out_adjlist = serialize_table.call(out_adjlist);
    std::string str_out_adjlist_inward = serialize_table.call(out_adjlist_inward);
    std::stringstream str_builder;
    str_builder << "adjlist = " << str_out_adjlist << ";\n adjlist_inward = " << str_out_adjlist_inward << ";";
    return str_builder.str();
}

std::string nodes_to_lua() {
    std::cout << "converting nodes" << std::endl;
    sol::state l;
    l.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
    l.script_file("scripts/io_module.lua");
    sol::function serialize_table = l["table_to_str"];
    std::stringstream out;
    out << "nodes = {\n";
    // this should traverse in order
    for (auto n : nodes) {
        int id = n.first;
        AudioProcessorNode *node = n.second;
        MiddleNode *real_node = dynamic_cast<MiddleNode *>(node);
        std::cout << "node: " << node->getId() << std::endl;
        if (real_node) {
            std::cout << (real_node->serializeLua()).get<std::string>("type") << std::endl;
            out << serialize_table.call<std::string>(real_node->serializeLua()) << ",\n";
        }
    }
    out << " };\n";
    return out.str();
}

void amp_save_preset(const std::string &file_path) {
    std::cout << "call" << std::endl;
    
}


void lua_to_adjlist(const sol::table &adjlist_p, const sol::table &adjlist_inward_p) {
    adjlist.clear();
    adjlist_inward.clear();
    for (const auto &o : adjlist_p) {
        sol::object index = o.first;
        sol::object value = o.second;
        if (index.get_type() == sol::type::number && value.get_type() == sol::type::number) {
            if ( nodes.find((index.as<int>() / 5) * 5) != nodes.end() && nodes.find((value.as<int>() / 5) * 5) != nodes.end()) {
                adjlist[index.as<int>()] = value.as<int>();
            }
        }
    }
    for (const auto &o : adjlist_inward_p) {
        sol::object index = o.first;
        sol::object value = o.second;
        if (index.get_type() == sol::type::number && value.get_type() == sol::type::number) {
            if ( nodes.find((index.as<int>() / 5) * 5) != nodes.end() && nodes.find((value.as<int>() / 5) * 5) != nodes.end()) {
                adjlist_inward[index.as<int>()] = value.as<int>();
            }
        }
    }
}

// call this before lua_to_adjlist otherwise you'll get a segfault
void lua_to_nodes(const sol::table &data) {
    // the first real node is id 10 since input node is id 0 and output onde is id 5
    for (const auto &p : nodes) {
        if (p.first != 0 && p.first != 5) {
            delete p.second;
            nodes.erase(p.first);
        }
    }
    int node_i = 10;
    for (const auto &o : data) {
        sol::object index = o.first;
        sol::object value = o.second;
        if (index.get_type() == sol::type::number && value.get_type() == sol::type::table) {
            std::string node_type = value.as<sol::table>().get_or<std::string>("type", "invalid");
            sol::table node_state = value.as<sol::table>().get_or("state", sol::table());
            if (node_type == "Overdrive") {
                nodes[node_i] = new guitar_amp::OverdriveNode(node_i, globalAudioInfo);
            } else if (node_type == "Oscillator") {
                nodes[node_i] = new guitar_amp::OscillatorNode(node_i, globalAudioInfo);
            } else if (node_type == "ThreeBandEQ") {
                nodes[node_i] = new guitar_amp::ThreeBandEQ(node_i, globalAudioInfo);
            } else if (node_type == "Flanger") {
                nodes[node_i] = new guitar_amp::FlangerNode(node_i, globalAudioInfo);
            } else if (node_type == "Delay") {
                nodes[node_i] = new guitar_amp::DelayNode(node_i, globalAudioInfo);
            } else if (node_type == "Convolution") {
                nodes[node_i] = new guitar_amp::ConvolutionNode(node_i, globalAudioInfo);
            } else if (node_type == "Compressor") {
                nodes[node_i] = new guitar_amp::CompressorNode(node_i, globalAudioInfo);                
            } else if (node_type == "CabSim") {
                nodes[node_i] = new guitar_amp::CabSimNode(node_i, globalAudioInfo);
            } else if (node_type == "Analyzer") {
                nodes[node_i] = new guitar_amp::AnalyzerNode(node_i, globalAudioInfo);
            } else {

            }
            MiddleNode *real_node = dynamic_cast<MiddleNode *>(nodes[node_i]);
            if (real_node) {
                real_node->luaInit(node_state);
            }
            ImNodes::SetNodeEditorSpacePos(node_i, ImVec2( 50 + (((node_i / 5)-2) % 3) * 350, 200 + ((((node_i / 5)-2) / 3) * 400)));
            node_i += 5;
        }  
    }
    ImNodes::SetNodeEditorSpacePos(5, ImVec2( 50 + ((((node_i / 5)-2) % 3) + 1) * 350, 200 + (((((node_i / 5)-2) / 3) + 1) * 200)));
    current_node = node_i;
}

void amp_load_preset(const std::string &file_name) {
    audioEnabled = false;
    nodes_mutex.lock();
    sol::state l;
    l.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
    l.script_file(file_name);
    lua_to_nodes(l.get_or("nodes", sol::table()));
    lua_to_adjlist(l.get_or("adjlist", sol::table()), l.get_or("adjlist_inward", sol::table()));
    audioEnabled = true;
    nodes_mutex.unlock();
}