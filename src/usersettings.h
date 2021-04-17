//
// Created by Ross Wardrup on 9/10/20.
//

#ifndef BEERTABS_USERSETTINGS_H
#define BEERTABS_USERSETTINGS_H

#include "ui_user_settings.h"
#include "mainwindow.h"

class UserSettings : public QDialog, public Ui::userSettingsDialog {
    /*
     * User settings dialog
     */

    Q_OBJECT
    Ui::userSettingsDialog ui{};
public:
    explicit UserSettings(QWidget *parent, const Options &options);
    std::string get_sex();
    std::string get_weekday_start();
    std::string get_date_calculation_method();
    int get_drink_limit();
    std::string get_limit_standard();
    std::string get_units();
    double get_std_drink_size();


private slots:
    void changed_limit_setting();
    void changed_date_calc();
    void clicked_clear_data();
};

#endif //BEERTABS_USERSETTINGS_H
