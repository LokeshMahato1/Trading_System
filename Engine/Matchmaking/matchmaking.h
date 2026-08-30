    #ifndef MATCHMAKING_H
    #define MATCHMAKING_H

    #include <vector>
    #include  "../order/order.h"


    struct MatchResult
    {
        bool matched;
        Order* buyOrder;
        Order* sellOrder;

        int matchedQuantity;
        double executionPrice;

        MatchResult()
        {
            matched = false;
            buyOrder = nullptr;
            sellOrder = nullptr;
            matchedQuantity = 0;
            executionPrice = 0.0;
        }

    };
    class MatchMaking {
        public:
            MatchMaking();
            
            bool isMatch(
                const Order& buyOrder, 
                const Order& sellOrder

            );

            MatchResult matchOrders(
                std::vector<Order>& buyOrders, 
                std::vector<Order>& sellOrders
            );

            int calculateMatchedQuantity(
                const Order& buyOrder,
                const Order& sellOrder
            ) const;

            double determineExecutionPrice(
                const Order& buyOrder,
                const Order& sellOrder
            ) const;
    };

    #endif 