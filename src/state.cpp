#include <state.hpp>
#include <sstream>
#include <MiddleNode.hpp>


std::string adjlist_to_lua() {
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
    sol::state l;
    l.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
    l.script_file("scripts/io_module.lua");
    sol::function serialize_table = l["table_to_str"];
    std::stringstream out;
    out << "nodes = { ";
    // this should traverse in order
    for (auto n : nodes) {
        int id = n.first;
        AudioProcessorNode *node = n.second;
        MiddleNode *real_node = dynamic_cast<MiddleNode *>(node);
        if (real_node) {
            out << serialize_table.call<std::string>(real_node->serializeLua()) << ",\n";
        }
    }
    out << " };\n";
}


void lua_to_adjlist(const sol::table &adjlist_p, const sol::table &adjlist_inward_p) {
    // please include a size variable
}

// call this before lua_to_adjlist otherwise you'll get a segfault
void lua_to_nodes(const sol::table &data) {
    // the first real node is id 10 since input node is id 0 and output onde is id 5
    int node_i = 10;

}