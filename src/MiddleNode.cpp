#include <MiddleNode.hpp>

guitar_amp::MiddleNode::MiddleNode(int id, const AudioInfo current_audio_info) : AudioProcessorNode(id) {
    internal_info = current_audio_info;
}

guitar_amp::MiddleNode::~MiddleNode() {

}