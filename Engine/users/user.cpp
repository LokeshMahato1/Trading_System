#include"user.h"

User::User(
    int id,
    const std::string& name,
    const std::string& username,
    const std::string& password,
    UserRole role
) 
: id(id), 
name(name),
username(username),
password(password),
role(role), 
loggedIn(false),
enabled(true) 
{
}


//getter function
int User::getId() const {
    return id;
}

const std::string& User::getName() const {
    return name;
}

const std::string& User::getUsername() const {
    return username;
}


UserRole User::getRole() const {
    return role;
}

bool User::isLoggedIn() const {
    return loggedIn;
}

bool User::isEnabled() const {
    return enabled;
}


//helper function

bool User::isAdmin() const {
    return role == UserRole::ADMIN;
}

bool User::isClient() const {
    return role == UserRole::CLIENT;
}


//login/logout functions


bool User::login(
    const std::string& username, 
    const std::string& password
) {
    if(!enabled) {
        return false; // User is disabled, cannot log in
    }
    if(this->username == username && this->password == password) {
        loggedIn = true;
        return true;
    }
    loggedIn = false;
    return false;
}

void User::logout() {
    loggedIn = false;
}

bool User::changePassword(const std::string& oldPassword, const std::string& newPassword) {

    if(password!=oldPassword) {
        return false; // Old password does not match
    }
    password = newPassword;
    return true;

}

//setter functions

void User::setName(const std::string& name) {
    this->name = name;
}

void User::setUsername(const std::string& username) {
    this->username = username;
}

void User::setPassword(const std::string& password) {
    this->password = password;
}

void User::setEnabled(bool status) {
    enabled = status;
}

