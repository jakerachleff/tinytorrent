//
// Created by Jake Rachleff on 11/25/17.
//

#ifndef TINYTORRENT_SONGREADER_H
#define TINYTORRENT_SONGREADER_H

#include <string>

#include <boost/array.hpp>

class itemreader {
public:
    itemreader(std::string item_id)
    {
        std::string filepath_ = "~/tinytorrent/" + item_id;
    }

    size_t read(boost::array<char, 1024>& buf)
    {
        return 10;
    }
private:
};


#endif //TINYTORRENT_SONGREADER_H
