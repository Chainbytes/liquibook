#include "example_order_book.h"

namespace liquibook { namespace examples {

ExampleOrderBook::ExampleOrderBook(const std::string& symbol)
: symbol_(symbol)
{

	
	//h.connect("http://127.0.0.1:3005");
}

const std::string&
ExampleOrderBook::symbol() const
{
  return symbol_;
}

//ищем книгу и ищем ордер
bool ExampleOrderBook::findExistingOrder(const std::string & orderId, OrderPtr & order)
{
	auto orderPosition = orders_.find(orderId);
	if (orderPosition == orders_.end())
	{
		std::cerr << "Orderbook: Can't find OrderID #" << orderId << std::endl;
		return false;
	}

	order = orderPosition->second;

	
	return true;
}


void ExampleOrderBook::on_accept(const OrderPtr& order, liquibook::book::Quantity quantity)
{
	order->onAccepted();
}

void ExampleOrderBook::on_fill(const OrderPtr& order,
	const OrderPtr& matched_order,
	liquibook::book::Quantity fill_qty,
	liquibook::book::Cost fill_cost,
	bool inbound_order_filled,
	bool matched_order_filled)
{
	order->onFilled(fill_qty, fill_cost);
	matched_order->onFilled(fill_qty, fill_cost);
}

void ExampleOrderBook::on_cancel(const OrderPtr& order, liquibook::book::Quantity quantity)
{
	order->onCancelled();
}




void ExampleOrderBook::on_replace(const OrderPtr& order,
	liquibook::book::Quantity current_qty,
	liquibook::book::Quantity new_qty,
	liquibook::book::Price new_price)
{
	order->onReplaced(new_qty - current_qty, new_price);
}

void ExampleOrderBook::on_order_book_change()
{

}

} } // End namespace

