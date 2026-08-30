#include "collateral.h"

Collateral::Collateral(int clientId, double initialCollateral)
    : clientId(clientId), availableCollateral(initialCollateral), blockedCollateral(0.0) {}


    //getters
int Collateral::getClientId() const {
    return clientId;
}

double Collateral::getAvailableCollateral() const {
    return availableCollateral;
}

double Collateral::getBlockedCollateral() const {
    return blockedCollateral;
}

double Collateral::getTotalCollateral() const {
    return availableCollateral + blockedCollateral;
}

//deposit 

bool Collateral::deposit(double amount) {
    if (amount <= 0) {
        return false; // Invalid deposit amount
    }
    availableCollateral += amount;
    return true;
}

//withdraw

bool Collateral::withdraw(double amount) {
    if (amount <= 0) {
        return false; // Invalid withdrawal amount
    }

    if (amount > availableCollateral) {
        return false; // Insufficient available collateral
    }
    availableCollateral -= amount;
    return true;
}

bool Collateral::reserve(double amount) {
    if (amount <= 0) {
        return false; // Invalid reserve amount
    }

    if (amount > availableCollateral) {
        return false; // Insufficient available collateral
    }

    availableCollateral -= amount;
    blockedCollateral += amount;
    return true;
}

//release collateral

bool Collateral::release(double amount) {
    if (amount <= 0) {
        return false; // Invalid release amount
    }

    if (amount > blockedCollateral) {
        return false; // Insufficient blocked collateral
    }
    blockedCollateral -= amount;
    availableCollateral += amount;
    return true;
}

//deduct collateral

bool Collateral::deduct(double amount) {
    if (amount <= 0) {
        return false; // Invalid deduction amount
    }

    if (amount > blockedCollateral) {
        return false; // Insufficient blocked collateral
    }
    blockedCollateral -= amount;
    return true;
}