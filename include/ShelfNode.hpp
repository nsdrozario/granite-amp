#ifndef GUITAR_AMP_SHELFNODE_HPP
#define GUITAR_AMP_SHELFNODE_HPP

#include "MiddleNode.hpp"
#include "internal_dsp.hpp"
#include "state.hpp"
#include "miniaudio.h"

namespace guitar_amp {
    
    class ShelfConfigBuilder {
    public:
        ShelfConfigBuilder(float g, float s, float f);
        float slope;
        float frequency;
        float gain;

        ma_hishelf2_config hishelf_cfg();
        ma_loshelf2_config loshelf_cfg();
    }; 

    class ShelfNode : public MiddleNode {
    public:
        ShelfNode(int id);
        ~ShelfNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames);

    private:
        ma_hishelf2 high_shelf;
        ma_loshelf2 low_shelf;

        ma_hishelf2_config high_shelf_cfg;
        ma_loshelf2_config low_shelf_cfg;

        /*
        0 = low shelf
        1 = high shelf
        */
        int shelf_type = 0;

    };
}

#endif