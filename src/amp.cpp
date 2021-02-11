/*
    Real time guitar amplifier simulation
    Copyright (C) 2021  Nathaniel D'Rozario

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MINIAUDIO_IMPLEMENTATION

#define MA_NO_DECODING
#define MA_NO_ENCODING

extern "C" {
    #include "miniaudio.h"
}

#include <kfr/all.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imnodes.h"

#include <AudioProcessorNode.hpp>
#include <InputNode.hpp>
#include <OutputNode.hpp>
#include <OverdriveNode.hpp>

#include <utility>
#include <set>
#include <unordered_set>

#define AMP_SAMPLE_RATE 48000

#include <internal_dsp.hpp>
#include <state.hpp>
using namespace guitar_amp;

int main () {

    // Initialize ImGui
    sf::Event e;
    sf::RenderWindow w(sf::VideoMode(800,600), "Guitar Amp");
    sf::Clock dt;
    ImGui::SFML::Init(w);
    imnodes::Initialize();

    /*
        Each node will reserve 5 times it's own zero-indexed ID, plus 4 more spaces.
        The first two IDs after the node's ID * 5 are reserved for input, and the next two for output.
        For example:
        Node 0:
            ID: 0
            Input: 1, 2
            Output: 3, 4
        Node 1:
            ID: 5
            Input: 6, 7
            Output: 8, 9
    */
    std::vector<AudioProcessorNode *> nodes;
    std::vector<std::unordered_set<int>> adjlist_outward;
    std::vector<std::unordered_set<int>> adjlist_inward;
    std::vector<std::pair<int,int>> edge_list;

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

        // imgui stuff

        imnodes::BeginNodeEditor();

        
        for (size_t i = 0; i < adjlist_outward.size(); i++) {
            int starting_node = (i/static_cast<size_t>(5))*static_cast<size_t>(5);
            for (auto t : adjlist_outward[i]) {
                imnodes::Link(starting_node, i, t);
            }
        } 

        

        imnodes::EndNodeEditor();

       


        int start_link; int end_link;
        if (imnodes::IsLinkCreated(&start_link, &end_link)) {
            adjlist_outward[start_link].insert(end_link);
            adjlist_inward[end_link].insert(start_link);
            edge_list.push_back(std::pair<int,int>(start_link,end_link));
        }

        int destroyedLink;
        if (imnodes::IsLinkDestroyed(&destroyedLink)) {
            adjlist_outward[edge_list[destroyedLink].first].erase(edge_list[destroyedLink].second);
            adjlist_inward[edge_list[destroyedLink].second].erase(edge_list[destroyedLink].first);
        }

        ImGui::Begin("I/O Devices");
            
        ImGui::End();


        // raw sfml calls

        ImGui::SFML::Render(w);
        w.display();
    }

    return 0;
}

