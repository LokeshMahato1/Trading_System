#include "matchmaking.h"
#include <algorithm>

MatchMaking::MatchMaking() {}

//check if two orders can be matched based on their prices and quantities

bool MatchMaking::isMatch(
    const Order& buyOrder,
    const Order&sellOrder
    )

    {
        // Check if the stock symbols match
        if(buyOrder.getStockSymbol()!=sellOrder.getStockSymbol())
        {
            return false;
        }

        //prevent a client from matching with their own order
        if(buyOrder.getUserId()==sellOrder.getUserId())
        {
            return false;
        }   
        //check if the buy order price is greater than or equal to the sell order price
        if(buyOrder.getPrice()<sellOrder.getPrice())
        {
            return false;
        }
        //both orders must be active
        if(buyOrder.getStatus()==OrderStatus::CANCELLED || buyOrder.getStatus()==OrderStatus::FILLED || buyOrder.getStatus()==OrderStatus::FAILED)
        {
            return false;
        }
        if(sellOrder.getStatus()==OrderStatus::CANCELLED || sellOrder.getStatus()==OrderStatus::FILLED || sellOrder.getStatus()==OrderStatus::FAILED)
        {
            return false;
        }
        //Quantity must be avilable in both orders

        if(buyOrder.getRemainingQuantity()<=0)
        {
            return false;
        }
        if (sellOrder.getRemainingQuantity()<=0)
        {
            return false;
        }

        return true;
    }

int MatchMaking::calculateMatchedQuantity(
    const Order& buyOrder,
    const Order& sellOrder
) const
{
    return std::min(buyOrder.getRemainingQuantity(), sellOrder.getRemainingQuantity());
}

double MatchMaking::determineExecutionPrice(
    const Order& /*buyOrder*/,
    const Order& sellOrder
) const
{
    return sellOrder.getPrice();
}

MatchResult MatchMaking::matchOrders(
        std::vector<Order>& buyOrders,
        std::vector<Order>& sellOrders
    )
    {
        MatchResult result;

        for (Order& buy : buyOrders)
        {
            for (Order& sell : sellOrders)
            {
                if (!isMatch(buy, sell))
                {
                    continue;
                }
                result.matched=true;
                result.buyOrder=&buy;
                result.sellOrder=&sell;
                result.matchedQuantity=calculateMatchedQuantity(buy, sell);

                //execute at resting sell order price 
                result.executionPrice=determineExecutionPrice(buy, sell);
                return result;
            }
        }
        return result;
    }
    
        