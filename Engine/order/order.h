#ifndef ORDER_H
#define ORDER_H


#include <string>

enum class OrderType {
    BUY,
    SELL
};

enum class OrderStatus{
    PENDING =0, 
    PARTIALLY_FILLED =1,
    FILLED =2,
    FAILED =3,
    CANCELLED =4
};


class Order 
{
    private:
        int orderId;
        int userId;

        std::string stockSymbol;

        int quantity;
        int remainingQuantity;

        double price;

        OrderType type;
        OrderStatus status;

        std::string createdAt;


    public:
        Order();

        Order(
            int orderId, 
            int userId, 
            const std::string& stockSymbol, 
            int quantity, 
            double price, 
            OrderType type
        );
        //getters

        int getOrderId() const;
        int getUserId() const;
        std::string getStockSymbol() const; 
        int getQuantity() const;
        int getRemainingQuantity() const;
        double getPrice() const;
        OrderType getType() const;
        OrderStatus getStatus() const;
        const std::string& getCreatedAt() const;

        //setters

        void setRemainingQuantity(int remainingQuantity);
        void setStatus(OrderStatus status);
        void setCreatedAt(const std::string& createdAt);
        void setPrice(double price);

        //utility functions

        bool isFilled() const;
        bool isCancelled() const;
        bool isPending() const;
        bool isBuyOrder() const;
        bool isSellOrder() const;



};
#endif