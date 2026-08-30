#include "order.h"

//default constructor

Order::Order()
{
    orderId=0;
    userId=0;
    stockSymbol=""; 
    quantity=0;
    remainingQuantity=0;
    price=0.0;
    type=OrderType::BUY;
    status=OrderStatus::PENDING;

    createdAt="";
}

//parameterized constructor

Order::Order(
    int orderId, 
    int userId, 
    const std::string& stockSymbol, 
    int quantity, 
    double price, 
    OrderType type)
{
    this->orderId=orderId;
    this->userId=userId;
    this->stockSymbol=stockSymbol; 
    this->quantity=quantity;
    this->remainingQuantity=quantity;
    this->price=price;
    this->type=type;
    this->status=OrderStatus::PENDING;
    this->createdAt="";
}

//getters

int Order::getOrderId()const
{
    return orderId;
}

int Order::getUserId()const
{
    return userId;
}


std::string Order::getStockSymbol()const
{
    return stockSymbol;
}


int Order::getQuantity()const
{
    return quantity;
}

int Order::getRemainingQuantity()const
{
    return remainingQuantity;
}


double Order::getPrice()const
{
    return price;
}

OrderType Order::getType() const
{
    return type;
}

OrderStatus Order:: getStatus() const 
{
    return status;
}

const std::string& Order::getCreatedAt() const
{
    return createdAt;
}

//setters

void Order::setRemainingQuantity(int remainingQuantity)
{
    this->remainingQuantity=remainingQuantity;
}

void Order::setStatus(OrderStatus status)
{
    this->status=status;
}

void Order::setCreatedAt(const std::string& createdAt)
{
    this->createdAt=createdAt;
}

void Order::setPrice(double price)
{
    this->price=price;
}

//utility functions

bool Order::isFilled() const
{
    return status==OrderStatus::FILLED;
}

bool Order::isCancelled() const
{
    return status==OrderStatus::CANCELLED;
}

bool Order::isPending() const
{
    return status==OrderStatus::PENDING;
}

bool Order::isBuyOrder() const
{
    return type==OrderType::BUY;
}

bool Order::isSellOrder() const
{
    return type==OrderType::SELL;
}   



