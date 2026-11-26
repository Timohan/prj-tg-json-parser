/*!
 * \file
 * \brief file main.cpp
 *
 * main of prj_tg_json_parser_test
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#include <algorithm>
#include <vector>
#include <filesystem>
#include <iostream>
#include "prj_tg_json_parser_test.h"

/*!
 * \brief main
 * \param argc
 * \param argv
 * \return
 */
int main(int argc , char *argv[])
{
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(ORDERS_FOLDER)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            files.push_back(entry.path().string());
        }
    }
    if (files.empty()) {
        std::cout << "No orders files found in: " << ORDERS_FOLDER << "\n";
        return 1;
    }
    std::sort(files.begin(), files.end());
    for (const std::string &filename : files) {
        PrjTgHJsonParserTest test;
        if (!test.test(filename)) {
            std::cout << "Failed test orders file: " << filename << "\n";
            return 1;
        }
    }
    std::cout << "All " << files.size() << " orders files passed\n";
    return 0;
}
