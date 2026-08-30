#ifndef USER_H
#define USER_H

#include <string>

enum class UserRole {
   CLIENT,
    ADMIN
}; 

class User
{
    protected:
        int id;
        std::string name;
        std::string username;
        std::string password;
        UserRole role;

        bool loggedIn;
        bool enabled;

        public:
            User(
                int id,
                const std::string& name,
                const std::string& username,
                const std::string& password,
                UserRole role
            );

            virtual ~User() = default;

            //getters function returns the value of the private member variables
        int getId() const;
        const std::string& getName() const;
        const std::string&  getUsername() const;
        UserRole getRole() const;
        bool isLoggedIn() const;
        bool isEnabled() const;

        //helpers function to login and logout the user

        bool isAdmin() const ;
        bool isClient() const ;
        

        //login and logout functions to manage user authentication
        bool login(
            const std::string& username, 
            const std::string& password
        );
        void logout();
        bool changePassword(const std::string& oldPassword, const std::string& newPassword);


//setters function sets the value of the private member variables
        void setName(const std::string& name);
        void setUsername(const std::string& username);
        void setPassword(const std::string& password);
        void setEnabled(bool status);

        


        //pure virtual function to display the user menu, which must be implemented by derived classes
        virtual void displayMenu() = 0;
       

        

       

};
#endif