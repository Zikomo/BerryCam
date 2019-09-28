//
// Created by Zikomo Fields on 2019-09-28.
//

#ifndef BERRYCAM_BROADCASTER_H
#define BERRYCAM_BROADCASTER_H


namespace BerryCam {
    class Broadcaster {
    public:
        virtual void SendPacket(unsigned char *data, int size) = 0;
    };
}


#endif //BERRYCAM_BROADCASTER_H
