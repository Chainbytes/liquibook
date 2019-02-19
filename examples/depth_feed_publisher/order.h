#ifndef example_order_h
#define example_order_h

#include "book/order.h"
#include <iostream>
#include <book/types.h>
#include <boost/signals2.hpp>



#include <string>
#include <vector>

namespace liquibook { namespace examples {

class Order : public book::Order {
public:

	enum State{
		Submitted,
		Rejected, // Terminal state
		Accepted,
		ModifyRequested,
		ModifyRejected,
		Modified,
		PartialFilled,
		Filled, // Terminal State
		CancelRequested,
		CancelRejected,
		Cancelled, // Terminal state
		Unknown
	};

	struct StateChange
	{
		State state_;
		std::string description_;
		StateChange()
			: state_(Unknown)
		{}

		StateChange(State state, const std::string & description = "")
			: state_(state)
			, description_(description)
		{}
	};
	typedef std::vector<StateChange> History;

	boost::signals2::signal<void(std::string, int32_t, double, int)> signalOrderChanged; //отправка статуса в сокет когда происходит изменение ордера===

	Order & verbose(bool verbose = true);
	bool isVerbose()const;
	const History & history() const;
	const StateChange & currentState() const;
	void sendStatusToSocket();

	std::string getStringState();

	uint32_t quantityFilled() const;

	uint32_t quantityOnMarket() const;

	uint32_t fillCost() const;

	///////////////////////////
	// Order life cycle events
	void onSubmitted();
	void onAccepted();
	void onRejected(const char * reason);

	void onFilled(
		liquibook::book::Quantity fill_qty,
		liquibook::book::Cost fill_cost);

	void onCancelRequested();
	void onCancelled();
	void onCancelRejected(const char * reason);

	void onReplaceRequested(
		const int32_t& size_delta,
		liquibook::book::Price new_price);

	void onReplaced(const int32_t& size_delta,
		liquibook::book::Price new_price);

	void onReplaceRejected(const char * reaseon);






  Order(bool buy,
        const double& price,          
		book::Quantity qty, std::string ClOrdID, std::string status);

  virtual bool is_buy() const;
  virtual book::Quantity order_qty() const;
  virtual book::Price price() const;
  virtual std::string order_ClOrdID() const;
  

  static const uint8_t precision_;
private:
  bool is_buy_;
  double price_;
  book::Quantity qty_;
  std::string ClOrdID_;
  std::string status_;
  std::vector<StateChange> history_;
  bool verbose_;

  liquibook::book::Quantity quantityFilled_=0;
  int32_t quantityOnMarket_=0;
  uint32_t fillCost_=0;
};

std::ostream & operator << (std::ostream & out, const Order & order);
std::ostream & operator << (std::ostream & out, const Order::StateChange & event);

}



}

#endif
