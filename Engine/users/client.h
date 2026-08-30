#ifndef CLIENT_H
#define CLIENT_H

#include "user.h"
#include <memory>
#include "../portfolio/portfolio.h"
#include "../collateral/collateral.h"

class Client : public User
{

    private:
        Portfolio portfolio;
        Collateral collateral;
    
    public:
        Client(
            int id,
            const std::string& name,
            const std::string& username,
            const std::string& password
        );

        bool placeBuyOrder(
            const std::string& stockSymbol,
            int quantity,
            double price
        );

        bool placeSellOrder(
            const std::string& stockSymbol,
            int quantity,
            double price
        );

        bool cancelOrder(int orderId);

        bool modifyOrder(int orderId, int newQuantity, double newPrice);

        bool depositCollateral(double amount);
        bool withdrawCollateral(double amount);

        void viewPortfolio() const;
        void viewCollateral() const;

    //getters
        Portfolio& getPortfolio();
        const Portfolio& getPortfolio()const;

        Collateral& getCollateral();
        const Collateral& getCollateral()const;

        //Display function
        void displayMenu() override;
        ~Client() override = default;


};
#endif