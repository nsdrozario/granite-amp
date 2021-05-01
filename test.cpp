#include <iostream>
#include <map>
#include <set>
#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imnodes.h"

using std::map;
using std::set;
using std::cout;

int main () {

    sf::RenderWindow w ({800,600}, "test");
    sf::Event e;
    sf::Clock dt;
    ImGui::SFML::Init(w);
    imnodes::Initialize();

    map<int,int> adjlist;
    map<int,int> adjlist_backwards;
    set<int> nodes;

    /*
    for each node n, n >= 0,
    let node id = 3n
    let node input = 3n + 1
    let node output = 3n + 2
    */

    nodes.insert(0);
    nodes.insert(1);
    nodes.insert(2);

    adjlist[2] = 4;
    adjlist_backwards[4] = 2;
    adjlist[5] = 7;
    adjlist_backwards[7] = 5;

    while (w.isOpen()) {
        while (w.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(e);
            if (e.type == sf::Event::Closed) {
                imnodes::Shutdown();
                ImGui::SFML::Shutdown();
                w.close();
            }
        }
        w.clear();
        ImGui::SFML::Update(w, dt.restart());
        
        imnodes::BeginNodeEditor();
            for (int i : nodes) {
                imnodes::BeginNode(i);
                    ImGui::Text(std::to_string(i).c_str());
                    imnodes::BeginInputAttribute((3 * i) + 1);
                    imnodes::EndInputAttribute();
                    imnodes::BeginOutputAttribute((3*i)+2);
                    imnodes::EndOutputAttribute();
                imnodes::EndNode();
            }
            for (auto p : adjlist) {
                if (p.second != 0) {
                    imnodes::Link(p.first/3, p.first, p.second);
                }
            }
        imnodes::EndNodeEditor();
        int hovered_node;
        if (imnodes::IsNodeHovered(&hovered_node) && ImGui::IsMouseReleased(ImGui::ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("Delete Node");
        }

        if (ImGui::BeginPopup("Delete Node")) {
            if (ImGui::MenuItem("Delete")) {
                nodes.erase(hovered_node);
                
            }
            ImGui::EndPopup();
        }

        int start, end;
        if (imnodes::IsLinkCreated(&start, &end)) {
            adjlist[start] = end;
        }

        int link_id;
        if (imnodes::IsLinkDestroyed(&link_id)) {
            cout << "link destroyed\n";
        }

        ImGui::SFML::Render(w);
        w.display();
    }

    return 0;

}