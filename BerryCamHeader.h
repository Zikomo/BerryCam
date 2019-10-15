//
// Created by Zikomo Fields on 10/13/19.
//

#ifndef BERRYCAM_BERRYCAMHEADER_H
#define BERRYCAM_BERRYCAMHEADER_H

#define BERRYCAM_PREAMBLE "Lucky Samurai"

namespace BerryCam {
    struct BerryCamHeader {
        char preamble[14];
        unsigned int size;
    };
}

#endif //BERRYCAM_BERRYCAMHEADER_H
