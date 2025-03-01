//
// Created by Ross Wardrup on 9/19/20.
//

#include <utility>
#include "mainwindow.h"
#include "calculate.h"

void MainWindow::update_wine_fields() {
    /*
     * Read rows in the DB and populate the winery, type, and name dropdowns with unique values.
     */

    std::set<QString> wineries;
    std::set<QString> types;
    std::set<QString> subtypes;
    std::vector<std::string> names_tmp;
    std::set<std::string> names;

    std::vector<Drink> all_wine = Database::filter("Alcohol Type", "Wine", storage);

    // Block signals to avoid crashing
    QSignalBlocker winery_signal_blocker(ui->wineryInput);
    QSignalBlocker type_signal_blocker(ui->wineTypeInput);
    QSignalBlocker subtype_signal_blocker(ui->wineSubtypeInput);
    QSignalBlocker name_signal_blocker(ui->wineNameInput);

    ui->wineryInput->clear();
    ui->wineryInput->setCurrentText("");
    ui->wineTypeInput->clear();
    ui->wineTypeInput->setCurrentText("");
    ui->wineSubtypeInput->clear();
    ui->wineSubtypeInput->setCurrentText("");
    ui->wineNameInput->clear();
    ui->wineNameInput->setCurrentText("");

    rename_duplicate_drink_names(all_wine);

    for (const auto& wine : all_wine) {
        const QString winery_name = QString::fromStdString(wine.get_producer());
        const QString wine_type = QString::fromStdString(wine.get_type());
        const QString wine_subtype = QString::fromStdString(wine.get_subtype());
        const std::string wine_name = wine.get_name();

        wineries.insert(winery_name);
        types.insert(wine_type);
        subtypes.insert(wine_subtype);
        names.insert(wine_name);
    }

    for (const auto& winery : wineries) {
        if (!winery.isEmpty()) {
            ui->wineryInput->addItem(winery);
        }
    }

    for (const auto& type : types) {
        if (!type.isEmpty()) {
            ui->wineTypeInput->addItem(type);
        }
    }

    for (const auto& name : names) {
        if (!name.empty()) {
            if (std::find(names_tmp.begin(), names_tmp.end(), name) == names_tmp.end())
                names_tmp.push_back(name);
        }
    }

    std::sort(names_tmp.begin(), names_tmp.end(), Calculate::compare_strings);

    for (const auto& name : names_tmp) {
        const QString name_q = QString::fromStdString(name);
        ui->wineNameInput->addItem(name_q);
    }

    for (const auto& subtype : subtypes) {
        if (!subtype.isEmpty()) {
            ui->wineSubtypeInput->addItem(subtype);
        }
    }

    // Rest to first name in field
    ui->wineNameInput->setCurrentIndex(0);
    update_wine_types_producers();
    const std::string wine_notes_text = get_latest_notes(ui->wineNameInput->currentText().toStdString());
    ui->wineNotesInput->setText(QString::fromStdString(wine_notes_text));
}

void MainWindow::populate_wine_fields(const Drink& drink_at_row) {
    /*
     * Populate the wine entry fields if user is on the wine entry tab.
     */

    const QDate date = format_date_for_input(drink_at_row);
    const std::string notes = get_latest_notes(drink_at_row.get_name());
    ui->wineDateInput->setDate(date);
    ui->wineNameInput->setCurrentText(drink_at_row.get_name().c_str());
    ui->wineTypeInput->setCurrentText(drink_at_row.get_type().c_str());
    ui->wineSubtypeInput->setCurrentText(drink_at_row.get_subtype().c_str());
    ui->wineryInput->setCurrentText(drink_at_row.get_producer().c_str());
    ui->wineVintage->setValue(drink_at_row.get_vintage());
    ui->wineAbvInput->setValue(drink_at_row.get_abv());
    ui->wineSizeInput->setValue(drink_at_row.get_size());
    ui->wineRatingInput->setValue(drink_at_row.get_rating());
    ui->wineNotesInput->setText(notes.c_str());

    // Switch to the liquor tab
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::update_wine_types_producers() {
    /*
     * Update wine type and winery when name input changes.
     */

    // This fixes crashes when changing with rows selected.
    QSignalBlocker type_input_signal_blocker(ui->wineTypeInput);
    QSignalBlocker subtype_input_signal_blocker(ui->wineSubtypeInput);
    QSignalBlocker brewery_input_signal_blocker(ui->wineryInput);

    const std::string input_wine = ui->wineNameInput->currentText().toStdString();
    Drink selected_wine;
    if (input_wine.find(" -- (") != std::string::npos) {  // This is a drink with a name that matches another beer, and contains the producer name in the dropdown
        std::string producer_name {input_wine.substr(input_wine.find(" -- (") + 5)};
        producer_name = producer_name.substr(0, producer_name.size() - 1);
        const std::string wine_name {input_wine.substr(0, input_wine.find(" -- ("))};
        selected_wine = Database::get_drink_by_name(storage, "Wine", wine_name, producer_name);
    } else {
        selected_wine = Database::get_drink_by_name(storage, "Wine", input_wine);
    }

    if (!selected_wine.get_id() || selected_wine.get_id() == -1) {
        clear_fields("Wine");
    } else {
        const std::string wine_type = selected_wine.get_type();
        const std::string wine_subtype = selected_wine.get_subtype();
        const std::string producer = selected_wine.get_producer();
        const double abv = selected_wine.get_abv();
        double size = selected_wine.get_size();
        const int vintage {selected_wine.get_vintage()};

        if (options.units == "Metric") {
            size = Calculate::oz_to_ml(size);
        }

        const int rating = selected_wine.get_rating();
        ui->wineTypeInput->setCurrentText(QString::fromStdString(wine_type));
        ui->wineSubtypeInput->setCurrentText(QString::fromStdString(wine_subtype));
        ui->wineryInput->setCurrentText(QString::fromStdString(producer));
        ui->wineAbvInput->setValue(abv);
        ui->wineSizeInput->setValue(size);
        ui->wineRatingInput->setValue(rating);
        ui->wineVintage->setValue(vintage);

        // Set notes to the notes for liquor in the name input
        const std::string notes = get_latest_notes(ui->wineNameInput->currentText().toStdString());
        ui->wineNotesInput->setText(QString::fromStdString(notes));
    }
}

Drink MainWindow::get_wine_attrs_from_fields(std::string alcohol_type) {
    /*
     * Get wine attributes from user input
     */

    Drink drink;

    std::string drink_name {ui->wineNameInput->currentText().toStdString()};

    if (drink_name.find(" -- (") != std::string::npos) {  // This is a beer with a name that matches another beer, and contains the producer name in the dropdown
        const std::string producer_name {drink_name.substr(drink_name.find(" -- (") + 5)};
        drink_name = drink_name.substr(0, drink_name.find(" -- ("));
    }

    const std::string drink_date = ui->wineDateInput->date().toString("yyyy-MM-dd").toStdString();
    drink.set_date(drink_date);
    drink.set_name(drink_name);
    drink.set_type(ui->wineTypeInput->currentText().toStdString());
    drink.set_subtype(ui->wineSubtypeInput->currentText().toStdString());
    drink.set_producer(ui->wineryInput->currentText().toStdString());
    drink.set_vintage(ui->wineVintage->value());
    drink.set_abv(ui->wineAbvInput->value());
    drink.set_ibu(-1.0);  // -1 denotes no IBU value
    drink.set_size(ui->wineSizeInput->value());
    drink.set_rating(ui->wineRatingInput->value());
    drink.set_notes(ui->wineNotesInput->toPlainText().toStdString());
    drink.set_alcohol_type(alcohol_type);

    return drink;
}
