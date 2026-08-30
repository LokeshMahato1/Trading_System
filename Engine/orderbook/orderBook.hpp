#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <vector>
#include "../order/order.h"  

class OrderBook{
    private:
        std::vector<Order> buyOrders;
        std::vector<Order> sellOrders;

        void sortBuyOrders();
        void sortSellOrders();

    public:
        OrderBook();

        //order operations

        bool addOrder(const Order& order);
        bool cancelOrder(int orderId);
        bool modifyOrder(int orderId, int newQuantity, double newPrice);

        //search and retrieval

        Order* findOrder(int orderId);

        const Order* findOrder(int orderId) const;

        //accessors

        const std::vector<Order> getBuyOrders() const;
        const std::vector<Order> getSellOrders() const;

        size_t buySize() const;
        size_t sellSize() const;

        bool empty() const;

        void clear();
};

#endif