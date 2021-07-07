#ifndef GUITAR_AMP_EQNODE_HPP
#define GUITAR_AMP_EQNODE_HPP

#include "MiddleNode.hpp"
#include "state.hpp"
#include <list>
#include <utility>
#include "internal_dsp.hpp"

namespace guitar_amp {
    class EQNode : public MiddleNode {
    public:
        EQNode(int id);
        ~EQNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, const AudioInfo &info);
    private:
        std::list<ma_peak2> filters;
        ma_hishelf2 hishelf;
        ma_loshelf2 loshelf;
        ma_lpf2 lpf;
        ma_hpf2 hpf; 
        bool enabled_hpf = false;
        bool enabled_lpf = false;
        bool enabled_hishelf = false;
        bool enabled_loshelf = false;
        std::vector<std::pair<double, double>> points;
    };
}

#endif