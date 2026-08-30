#include "admin.h"

#include "../database/db.hpp"
#include "../market/market.h"

#include <iostream>

Admin::Admin(
    int id,
    const std::string& name,
    const std::string& username,
    const std::string& password
) : User(id, name, username, password, UserRole::ADMIN) {}

//user management functions

bool Admin::addUser(
    Database& db,
    const std::string& name,
    const std::string& username,
    const std::string& password,
    const std::string& role
) {
    return db.addUser(name, username, password, role);
}

bool Admin::removeUser(Database& db, int userId) {
    return db.removeUser(userId);
}

bool Admin::resetPassword(Database& db, int userId, const std::string& newPassword) {
    return db.changePassword(userId, newPassword);
}

//market management functions

void Admin::openMarket(Market& market) {
    market.setMarketStatus(true);
}

void Admin::closeMarket(Market& market) {
    market.setMarketStatus(false);
}




void Admin::displayMenu() {
    std::cout << "========== Admin Menu ==========" << std::endl;
    std::cout << "1. View All Users" << std::endl;
    std::cout << "2. Add User" << std::endl;
    std::cout << "3. Remove User" << std::endl;
    std::cout << "4. Reset User Password" << std::endl;
    std::cout << "5.  View Orders" << std::endl;
    std::cout << "6. Open Market" << std::endl;
    std::cout << "7. Close Market" << std::endl;
    std::cout << "8. Logout" << std::endl;
}

