#ifndef example_order_book_h
#define example_order_book_h

#include "order.h"
#include "book/depth_order_book.h"
#include <boost/shared_ptr.hpp>





namespace liquibook { namespace examples {

typedef boost::shared_ptr<Order> OrderPtr;
typedef std::map<std::string, OrderPtr> OrderMap;

class ExampleOrderBook : public book::DepthOrderBook<OrderPtr>  {
public:
  ExampleOrderBook(const std::string& symbol =0);
  const std::string& symbol() const;
  bool findExistingOrder(const std::string & orderId, OrderPtr & order);
  /// @brief callback for an order accept
  virtual void on_accept(const OrderPtr& order, liquibook::book::Quantity quantity);
  virtual void on_fill(const OrderPtr& order,
	  const OrderPtr& matched_order,
	  liquibook::book::Quantity fill_qty,
	  liquibook::book::Cost fill_cost,
	  bool inbound_order_filled,
	  bool matched_order_filled);

  virtual void on_cancel(const OrderPtr& order, liquibook::book::Quantity quantity);

  virtual void on_replace(const OrderPtr& order,
	  liquibook::book::Quantity current_qty,
	  liquibook::book::Quantity new_qty,
	  liquibook::book::Price new_price);

  virtual void on_order_book_change();

  OrderMap orders_;

 // 
 // void sendJSONtoSocket(const OrderPtr& order);
 // JsonSender *jsonSender;
 
//  
  


private:
  std::string symbol_;
  
  //
  
};

} } // End namespace

#endif
