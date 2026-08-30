
#ifndef COLLATERAL_H
#define COLLATERAL_H

class Collateral
{
private:
    int clientId;
    double availableCollateral;
    double blockedCollateral;

public:
    Collateral(int clientId, double initialCollateral = 0.0);

    // Getters
    int getClientId() const;
    double getAvailableCollateral() const;
    double getBlockedCollateral() const;
    double getTotalCollateral() const;

    //operations

    bool deposit(double amount);
    bool withdraw(double amount);


    bool reserve(double amount);
    bool release(double amount);
    bool deduct(double amount);
};

#endif