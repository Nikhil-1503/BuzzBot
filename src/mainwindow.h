#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "database.h"
#include "../include/qcustomplot.h"
#include "../ui/ui_mainwindow.h"
#include "options.h"
#include "drink_standards.h"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    Options options;  // Reads options from FS

private:
    Ui::MainWindow *ui;
    Storage storage = initStorage(utilities::get_db_path());

    void populate_filter_menus(const std::string &filter_type);

    void update_beer_fields();

    void update_stat_panel();

    void update_liquor_fields();

    void update_wine_fields();

    void rename_duplicate_drink_names(std::vector<Drink> &drinks);
    void update_drinks_this_week(double standard_drinks, const std::string& weekday_name);
    void update_standard_drinks_left_this_week(double std_drinks_consumed);
    double update_vol_alcohol_consumed_this_week(const std::vector<Drink>& drinks_this_week, const std::string& weekday_name);
    void update_volume_alcohol_remaining(double volume_alcohol_consumed);
    void update_favorite_producer(const std::string& drink_type);
    void update_favorite_drink(const std::string& drink_type);
    void update_favorite_type(const std::string& drink_type);
    void update_mean_abv(const std::string& drink_type);
    void update_mean_ibu(const std::string& drink_type);
    void update_types_and_producers();
    std::string get_latest_notes(const std::string& name);
    std::string get_current_tab();
    Drink get_drink_attributes_from_fields();
    void update_selected_row(QItemSelectionModel* select, Drink entered_drink);
    void add_new_row(Drink entered_drink);
    std::vector<std::set<QString>> generate_filter_item_sets();
    static QDate format_date_for_input(const Drink& drink);
    void populate_beer_fields(const Drink& drink_at_row);
    void populate_liquor_fields(const Drink& drink_at_row);
    void populate_wine_fields(const Drink& drink_at_row);
    void update_beer_types_producers();
    void update_liquor_types_producers();
    void update_wine_types_producers();
    Drink get_beer_attrs_from_fields(std::string alcohol_type);
    Drink get_liquor_attrs_from_fields(std::string alcohol_type);
    Drink get_wine_attrs_from_fields(std::string alcohol_type);
    void add_menubar_items();
    void configure_calendar();
    void configure_table();
    void add_slot_connections();
    void set_input_states();
    Drink get_drink_at_selected_row();
    void clear_fields(const std::string& alcohol_type);
    [[nodiscard]] std::chrono::weekday get_filter_weekday_start() const;
    std::tuple<std::chrono::year_month_day, std::string> get_filter_date();
    void update_std_drinks_today();
    static std::string format_date(std::chrono::year_month_day date);
    static std::string get_weekday_name(unsigned weekday_number);
    void open_graphs();
    double get_std_drink_size_from_options();

private slots:
    void submit_button_clicked();
    void reset_fields();
    void update_table();
    void populate_fields(const QItemSelection &, const QItemSelection &);
    void delete_row();
    void enable_filter_text(const QString&);
    void changed_filter_text(const QString&);
    void open_user_settings();
    static void open_about_dialog();
    void open_export_dialog();
    void open_std_drink_calculator() const;
    void reset_table_sort();
    void name_input_changed(const QString&);
    void tab_changed();
    void clicked_clear_button();
    void update_stats_if_new_day();
};

#endif // MAINWINDOW_H
