#include "exchange.h"

namespace liquibook { namespace examples {

Exchange::Exchange(ExampleOrderBook::TypedDepthListener* depth_listener,
                   ExampleOrderBook::TypedTradeListener* trade_listener)
: depth_listener_(depth_listener),
  trade_listener_(trade_listener)
{
}

void
Exchange::add_order_book(const std::string& sym)
{
  std::pair<OrderBookMap::iterator, bool> result;
  result = order_books_.insert(std::make_pair(sym, ExampleOrderBook(sym)));
  result.first->second.set_depth_listener(depth_listener_);
  result.first->second.set_trade_listener(trade_listener_);
 
}

void Exchange::func(std::string str)
{
	//отправл€ем
}

bool Exchange::add_order(const std::string& symbol, OrderPtr& order)
{
  OrderBookMap::iterator order_book = order_books_.find(symbol);
  if (order_book != order_books_.end())  //нашли книгу
  {
	  //провер€ем ордер по »ƒ на уникальность
	  auto orderPosition = order_book->second.orders_.find(order->order_ClOrdID());

	  if (orderPosition == order_book->second.orders_.end()) //не нашли=====
	  {//добавл€ем
		  order_book->second.add(order);
		  order_book->second.orders_[order->order_ClOrdID()] = order;
		  order_book->second.perform_callbacks();
		  return true;
	  }
	  else
	  {
		  //нашли.. 
		  return false;
	  }

    
  }
}


std::string Exchange::cancel_order_by_ClOrdID(const std::string& symbol, const std::string& ClOrdID)
{

	OrderPtr order;

	OrderBookMap::iterator order_book = order_books_.find(symbol); //сначала находим книгу
	if (order_book != order_books_.end())
	{
		if (!order_book->second.findExistingOrder(ClOrdID, order))
		{
			return "--Can't find OrderID #" + ClOrdID+"\n";
			//std::cerr << "--Can't find OrderID #" << ClOrdID << std::endl;
			//return false?
		}
		else
		{
			
			//out() << "Requesting Cancel: " << *order << std::endl;
			order_book->second.cancel(order);
			order_book->second.perform_callbacks();
			return "Exchange: Requesting Cancel: OrderID #" + ClOrdID + "\n";
		}
	}
}

Application::Order_struct Exchange::getStatusOrder(const std::string& symbol, const std::string& ClOrdID)
{
	OrderPtr order;

	OrderBookMap::iterator order_book = order_books_.find(symbol); //сначала находим книгу
	if (order_book != order_books_.end())
	{
		Application::Order_struct order_struct;
		if (!order_book->second.findExistingOrder(ClOrdID, order))
		{
			order_struct.ClOrdID = ClOrdID;
			order_struct.qty = -1;
			
			return order_struct;
			//std::cerr << "--Can't find OrderID #" << ClOrdID << std::endl;
			//return false?
		}
		else
		{

			order_struct.ClOrdID = ClOrdID;
			order_struct.qty = order->quantityOnMarket();
			order_struct.status = order->getStringState();
			order_struct.is_buy = order->is_buy();

			book::Price price = order->price();
			order_struct.price = price;


			//std::cerr << "Exchange::getStatusOrder: " << str;

			//std::cerr << "Exchange::getStatusOrder";
			//std::string  str = order->getStringState();
			return order_struct;
			//out() << "Requesting Cancel: " << *order << std::endl;
			
		}
	}
}


} } // End namespace


