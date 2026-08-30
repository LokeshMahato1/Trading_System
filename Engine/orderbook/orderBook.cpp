#include "orderBook.hpp"

#include <algorithm>


OrderBook::OrderBook() {}

void OrderBook::sortBuyOrders(){
    std::sort(
        buyOrders.begin(),
        buyOrders.end(),
        [](const Order& a, const Order& b) {
            if(a.getPrice() == b.getPrice()) {
                return a.getCreatedAt() < b.getCreatedAt(); // Sort by creation time if prices are equal
            }
            return a.getPrice() > b.getPrice(); // Sort in descending order
        }
    );
}

void OrderBook::sortSellOrders(){
    std::sort(
        sellOrders.begin(),
        sellOrders.end(),
        [](const Order& a, const Order& b) {
            if(a.getPrice() == b.getPrice()) {
                return a.getCreatedAt() < b.getCreatedAt(); // Sort by creation time if prices are equal
            }
            return a.getPrice() < b.getPrice(); // Sort in ascending order
        }
    );
}

bool OrderBook::addOrder(const Order& order)
{
    if(order.getType() == OrderType::BUY)
    {
        buyOrders.push_back(order);
        sortBuyOrders();
        return true;
    }
    else if(order.getType() == OrderType::SELL)
    {
        sellOrders.push_back(order);
        sortSellOrders();
        return true;
    }
    else
    {
        return false; // Invalid order type
    }
    return true;
}

Order* OrderBook::findOrder(int orderId)
{
    for(auto& order : buyOrders)
    {
        if(order.getOrderId() == orderId)
        {
            return &order;
        }
    }

    for(auto& order : sellOrders)
    {
        if(order.getOrderId() == orderId)
        {
            return &order;
        }
    }

    return nullptr; // Order not found
}

bool OrderBook::cancelOrder(int orderId)
{
    auto buyIterator=
    std::find_if(
        buyOrders.begin(),
        buyOrders.end(),
        [orderId](const Order& order) {
            return order.getOrderId() == orderId;
        }
    );

    if(buyIterator != buyOrders.end())
    {
        buyOrders.erase(buyIterator);
        return true;
    }

    auto sellIterator=
    std::find_if(
        sellOrders.begin(),
        sellOrders.end(),
        [orderId](const Order& order) {
            return order.getOrderId() == orderId;       
        }
    );

    if(sellIterator != sellOrders.end())
    {
        sellOrders.erase(sellIterator);
        return true;
    }

    return false; // Order not found
}

bool OrderBook::modifyOrder(
    int orderId, 
    int newQuantity, 
    double newPrice
)
{
        Order* order = findOrder(orderId);
    if(order==nullptr)
    {
        return false; // Order not found
    }
        order->setRemainingQuantity(newQuantity);
        order->setPrice(newPrice);

        // Re-sort the orders after modification
        if(order->getType() == OrderType::BUY)
        {
            sortBuyOrders();
        }
        else
        {
            sortSellOrders();
        }

        return true;
}

const std::vector<Order> OrderBook::getBuyOrders() const
{
    return buyOrders;
}

const std::vector<Order> OrderBook::getSellOrders() const
{
    return sellOrders;
}

size_t OrderBook::buySize() const
{
    return buyOrders.size();
}

size_t OrderBook::sellSize() const
{
    return sellOrders.size();
}

bool OrderBook::empty() const
{
    return buyOrders.empty() && sellOrders.empty();
}


void OrderBook::clear()
{
    buyOrders.clear();
    sellOrders.clear();
}
