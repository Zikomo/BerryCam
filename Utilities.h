//
// Created by Zikomo Fields on 2019-03-27.
//

#ifndef BERRYCAM_UTILITIES_H
#define BERRYCAM_UTILITIES_H

#include <boost/property_tree/ptree.hpp>

class Utilities {
public:
    template<class T>
    static T SafeGet(const std::shared_ptr<boost::property_tree::ptree>& settings, std::string key, T default_value)
    {
        auto iterator = settings->find(key);
        boost::optional<T> value = settings->get_optional<T>(key);
        if (!value) {
            settings->put<T>(key, default_value);
            return default_value;
        }
        return settings->get<T>(key);
    }

    template<class T>
    static T SafeGet(boost::property_tree::ptree& settings, std::string key, T default_value)
    {
        auto iterator = settings.find(key);
        boost::optional<T> value = settings.get_optional<T>(key);
        if (!value) {
            settings.put<T>(key, default_value);
            return default_value;
        }
        return settings.get<T>(key);
    }

};


#endif //BERRYCAM_UTILITIES_H
