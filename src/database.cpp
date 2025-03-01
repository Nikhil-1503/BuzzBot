#include "database.h"
#include "utilities.h"
#include <utility>
#include <iostream>
#include <filesystem>
#include <string>

using namespace sqlite_orm;

std::vector<Drink> Database::read(Storage storage) {
    /*
     * Read all rows from the database.
     * @return all_drinks A vector containing Drink, storing all rows in the database.
     */

    const std::vector<Drink> all_drinks = storage.get_all<Drink>();
    std::cout << "Read " + std::to_string(all_drinks.size()) + " drinks from DB" << std::endl;

    return all_drinks;
}

void Database::write_db_to_disk(Storage &storage) {
    /*
     * Flush in-memory database data to disk.
     */

    storage.sync_schema(true);
}

Storage Database::write(Drink drink, Storage storage) {
    /*
     * Write a row to the SQLite database.
     * @param drink: a drink
     * @return Storage: The storage instance
     */

    const std::string database_path = utilities::get_db_path();
    const int inserted_id = storage.insert(drink);
    drink.id = inserted_id;
    write_db_to_disk(storage);

    return storage;
}

void Database::truncate(Storage storage) {
    /*
     * Delete all rows from the sqlite database while retaining columns.
     * @param storage: a storage instance.
     */

    storage.remove_all<Drink>();
    write_db_to_disk(storage);
}

void Database::delete_row(Storage &storage, const int &row_num) {
    /*
     * Delete a specific row from the database.
     * @param storage: A storage instance
     * @param row_num: Integer denoting row number to delete. This corresponds to DB primary key.
     */

    storage.remove<Drink>(row_num);
}

Drink Database::read_row(const int &row_num, Storage &storage) {
    /*
     * Read a specific row from the database.
     * @param storage: A storage instance.
     * @param row_num: The row number that should be read.
     * @return drink: The results of the database query.
     */
    
    const auto drink = storage.get<Drink>(row_num);
    return drink;
}

void Database::update(Storage storage, const Drink& drink) {
    /*
     * Update a specific database row.
     * @param storage: A storage instance.
     * @param drink: The row to update. The ID of the drink must match the internal primary key on the DB.
     */

    storage.update(drink);
}

std::string Database::get_latest_notes(Storage &storage, const std::string& name, const std::string& alcohol_type) {
    /*
     * Get the last notes entered for a drink.
     * @param storage: A Storage instance.
     * @param name: Name of alcohol to retrieve notes for.
     * @param alcohol_type: Type of alcohol that Name is.
     * @return notes: A string containing notes entered for the name and alcohol type.
     */

    const std::vector<Drink> drinks = storage.get_all<Drink>(where(c(&Drink::name) == name && c(&Drink::alcohol_type) == alcohol_type));
    std::string notes;
    int temp_id = 0;
    for (const auto& drink_for_notes : drinks) {
        if (drink_for_notes.id > temp_id && drink_for_notes.alcohol_type == alcohol_type) {
            temp_id = drink_for_notes.id;
            if (!drink_for_notes.notes.empty()) {
                notes = drink_for_notes.notes;
            }
        }
    }

    return notes;
}


std::vector<Drink> Database::filter(const std::string& filter_type, const std::string& filter_text, Storage storage) {
    /*
     * Retrieve DB rows based on filter column and text.
     * @param filter_type: Column on which to filter.
     * @param filter_text: Text to search columns for.
     * @param storage: A storage instance.
     * @return filtered_drinks: The results of the database query.
     */

    std::vector<Drink> filtered_drinks;

    // Filter by name
    if (filter_type == "Name") {
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::name) == filter_text));
    } else if (filter_type == "Type") {
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::type) == filter_text));
    } else if (filter_type == "Subtype") {
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::subtype) == filter_text));
    } else if (filter_type == "Producer") {
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::producer) == filter_text));
    } else if (filter_type == "Alcohol Type") {
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::alcohol_type) == filter_text));
    } else if (filter_type == "After Date") {
        std::cout << "Filter date: " << filter_text << std::endl;
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::date) >= filter_text));

    } else if (filter_type == "Rating") {
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::rating) == filter_text));
    } else if (filter_type == "Name & Producer") {
        // Parse the -- (PRODUCER) text to strip out drink name and producer.
        std::string producer_name {filter_text.substr(filter_text.find(" -- (") + 5)};
        producer_name = producer_name.substr(0, producer_name.size() - 1);
        const std::string drink_name {filter_text.substr(0, filter_text.find(" -- ("))};
        std::cout << drink_name << " " << producer_name << std::endl;
        filtered_drinks = storage.get_all<Drink>(where(c(&Drink::name) == drink_name and c(&Drink::producer) == producer_name));
    } else {
        filtered_drinks = storage.get_all<Drink>();
    }

    return filtered_drinks;
}

Drink Database::get_drink_by_name(Storage &storage, const std::string &alcohol_type, const std::string &drink_name) {
    /*
     * Get a drink by its name.
     * @param storage: A Storage instance
     * @param drink_name: Name of drink to query
     * @return drink_by_name: A Drink matching the name.
     */

    std::vector<Drink> drink_by_name_result = storage.get_all<Drink>(where(c(&Drink::name) ==
            drink_name && c(&Drink::alcohol_type) == alcohol_type));
    Drink drink_by_name;

    std::sort(drink_by_name_result.begin(), drink_by_name_result.end(), compare_date);

    if (!drink_by_name_result.empty()) {
        drink_by_name = drink_by_name_result.at(drink_by_name_result.size() - 1);
    } else {
        drink_by_name.id = -1;
    }

    return drink_by_name;
}
Drink Database::get_drink_by_name(Storage &storage, const std::string &alcohol_type, const std::string &drink_name, const std::string &producer) {
    /*
     * Overloaded get_drink_by_name method that gets a drink by name and producer, if provided.
     * @param storage: a Storage instance
     * @param drink_name: name of drink to query
     * @param alcohol_type: The alcohol type to query
     * @param producer: The drink producer
     */

    std::vector<Drink> drink_by_name_result = storage.get_all<Drink>(where(c(&Drink::name)
            == drink_name && c(&Drink::alcohol_type) == alcohol_type &&
            c(&Drink::producer) == producer));
    Drink drink_by_name;

    std::sort(drink_by_name_result.begin(), drink_by_name_result.end(), compare_date);

    if (!drink_by_name_result.empty()) {
        drink_by_name = drink_by_name_result.at(drink_by_name_result.size() - 1);
    } else {
        drink_by_name.id = -1;
    }

    return drink_by_name;
}

std::vector<Drink> Database::get_drinks_by_type(Storage &storage, std::string drink_type) {
    /*
     * Get drinks by type.
     * @param storage: A storage instance.
     * @param drink_type: The type of drink to filter on.
     * @return: A vector of drinks that match drink_type.
     */
    const std::vector<Drink> drinks_by_type = storage.get_all<Drink>(where(c(&Drink::type) == std::move(drink_type)));
    return drinks_by_type;
}

std::vector<Drink> Database::get_drinks_by_producer(Storage &storage, std::string producer) {
    /*
     * Get drinks by producer.
     * @param storage: A storage instance.
     * @param producer: The producer name to filter on.
     * @return: A vector of drinks produced by a producer.
     */
    const std::vector<Drink> drinks_by_producer = storage.get_all<Drink>(where(c(&Drink::producer) == std::move(producer)));
    return drinks_by_producer;
}

int Database::get_version(Storage storage) { // NOLINT(performance-unnecessary-value-param)
    /*
     * Get the current database version.
     * @param storage: A storage instance.
     * @return: An integer denoting the current database version.
     */
    return storage.pragma.user_version();
}

int Database::increment_version(Storage storage, int current_version) {
    /*
     * Increment database version to current version. This is used to implement
     * database changes in place across versions
     * @param storage: A storage instance.
     * @param current_version: The version that the DB should be incremented to.
     * @return: An integer denoting new DB version, straight from the DB.
     */

    std::cout << "Using DB version " << storage.pragma.user_version() << std::endl;
    const int version = get_version(storage);
    if (version < 8 && current_version == 9) {  //version 9 implements the new size column and version 10 will remove the _size column
        std::cout << "*** Upgrading DB from version " << storage.pragma.user_version() <<  " to " << current_version << std::endl;

        if (version != 0) {
            std::cout << "Not dropping _size column until db version 10." << std::endl;
            /* TODO: At DB version 10, we will want to copy all values of _size to size column and then drop _size.
             * It will be necessary to update code in mainwindow.cpp (and probably elsewhere) to then use this new
             * size column.*/
        }
    }
    // Set to new version number
    storage.pragma.user_version(current_version);
    storage.sync_schema(true);

    return storage.pragma.user_version();
}

bool Database::compare_date(const Drink &a, const Drink &b) {
    /*
     * Determine if second date is greater than the first date.
     * @return: True if second date is more recent than the first date. Else, false.
     */

    const int a_year = std::stoi(a.get_date().substr(0, 4));
    const int a_month = std::stoi(a.get_date().substr(5, 7));
    const int a_day = std::stoi(a.get_date().substr(8, 9));
    const int b_year = std::stoi(b.get_date().substr(0, 4));
    const int b_month = std::stoi(b.get_date().substr(5, 7));
    const int b_day = std::stoi(b.get_date().substr(8, 9));

    if (a_year < b_year) {
        return true;
    } else if (a_year == b_year) {
        if (a_month < b_month) {
            return true;
        } else if (a_month == b_month && a_day < b_day) {
            return true;
        } else if (a_month == b_month && a_day == b_day && a.id < b.id) {
            return true;
        }
    }
    // Else:
    return false;
}

void Database::sort_by_date_id(std::vector<Drink> &drinks) {
    /*
     * Adds a sort column integer to database.
     */

    // First sort by entered date
    std::sort(drinks.begin(), drinks.end(), compare_date);
    // Now add sort order value
    int sort_order = 1;
    for (unsigned i = 0; i < drinks.size(); ++i) { // NOLINT(modernize-loop-convert)
        drinks[i].sort_order = sort_order;
        sort_order++;
    }
}


int Database::move_db(const std::string &current_path, const std::string &new_path) {
    /*
     * Move DB file if user selects a new get_db_path option.
     * @param: None
     * @return: 0 if move is successful, else 1
     */

    const std::string std_path{utilities::get_application_data_path() + "/buzzbot.db"};

    if (utilities::file_exists(std_path + ".bak")) {
        std::filesystem::remove(std_path + ".bak");
    }

    std::filesystem::copy(current_path, std_path + ".bak");

    if (!utilities::file_exists(new_path))
    {
        try {
            std::filesystem::copy(current_path, new_path);
            if (utilities::file_exists(current_path)) {
                std::filesystem::remove(current_path);
            }
            return 0;
        } catch (std::filesystem::filesystem_error &e) {
            std::cout << e.what() << std::endl;
            std::cout << "Restoring backup!" << std::endl;
            std::filesystem::copy(std_path + ".bak", std_path);
        }
    } else {
        std::cout << new_path << " already exists. Not overwriting with " << current_path << std::endl;
        return 2;
    }

    return 1;
}
