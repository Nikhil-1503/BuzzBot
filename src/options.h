//
// Created by rwardrup on 9/19/22.
//

#ifndef BUZZBOT_OPTIONS_H
#define BUZZBOT_OPTIONS_H

#include "utilities.h"
#include <string>

class Options {
public:
    bool custom_database{false};
    std::string database_path{utilities::get_application_data_path() +
                              "/buzzbot.db"}; // To be set once app is first run, as it is OS dependent
    std::string sex = "male";
    std::string date_calculation_method = "Fixed";
    std::string weekday_start = "Sunday";
    std::string limit_standard = "NIAAA";
    std::string units = "Imperial";
    std::string std_drink_country = "United States";
    std::string std_drink_size = "0.6";  // Store as ounces
    int weekly_limit = -1;

    Options();

// Methods
    int write_options() const;
};


#endif //BUZZBOT_OPTIONS_H
