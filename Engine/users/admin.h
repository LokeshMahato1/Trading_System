#ifndef ADMIN_H
#define ADMIN_H

#include "user.h"
#include <string>

class Database; // Forward declaration of Database class
class Market;   // Forward declaration of Market class
class Admin : public User
{
public:
    Admin(
        int id,
        const std::string& name,
        const std::string& username,
        const std::string& password
    );

    bool addUser(
        Database& db,
        const std::string& name,
        const std::string& username,
        const std::string& password,
        const std::string& role
    );

    bool removeUser(Database& db, int userId);

    bool resetPassword(Database& db, int userId, const std::string& newPassword);

    //market management functions

    void openMarket(Market& market);
    void closeMarket(Market& market);




   //display Admin menu
   
    void displayMenu() override;

    ~Admin() override=default;
};

#endif
