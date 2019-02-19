#include "order.h"
#include <sstream>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace liquibook { namespace examples {

const uint8_t Order::precision_(100);

Order::Order(bool buy, const double& price, book::Quantity qty, std::string ClOrdID, std::string status)
: is_buy_(buy),
  price_(price),
  qty_(qty),
  ClOrdID_(ClOrdID),
  status_(status)
{

	
}

bool
Order::is_buy() const
{
  return is_buy_;
}

book::Quantity Order::order_qty() const
{
  return qty_;
}

std::string Order::order_ClOrdID() const
{
	return ClOrdID_;
}


book::Price Order::price() const
{
  return book::Price(price_ * precision_);
}

const Order::History & Order::history() const
{
	return history_;
}

const Order::StateChange & Order::currentState() const
{
	return history_.back();
}


Order & Order::verbose(bool verbose)
{
	verbose_ = verbose;
	return *this;
}

bool
Order::isVerbose() const
{
	return verbose_;
}

void
Order::onSubmitted()
{
	std::stringstream msg;
	msg << (is_buy() ? "BUY " : "SELL ") << qty_ ;
	if (price_ == 0)
	{
		msg << "MKT";
	}
	else
	{
		msg << price_;
	}
	history_.emplace_back(Submitted, msg.str());

	std::cerr << "onSubmitted ";

	//sendStatusToSocket();
}

void
Order::onAccepted()
{
	
	quantityOnMarket_ = qty_;
	

	//status from db===
	if (status_ == "")
	{
		std::cerr << ClOrdID_ << " onAccepted\n";
		history_.emplace_back(Accepted);
		sendStatusToSocket();
	}
	else
	{
		if (status_ == "Processing") //Partitionaly filled
		{
			history_.emplace_back(PartialFilled, "");
			std::cerr << ClOrdID_ << " onFilled from DB\n";
		}

		if (status_ == "Pending") //Accepted
		{
			std::cerr << ClOrdID_ << " onAccepted from DB\n";
			history_.emplace_back(Accepted);
		}
	}
		
}

void
Order::onRejected(const char * reason)
{
	history_.emplace_back(Rejected, reason);
	//sendStatusToSocket();
}

void
Order::onFilled(
liquibook::book::Quantity fill_qty,
liquibook::book::Cost fill_cost)
{
	quantityOnMarket_ -= fill_qty;
	fillCost_ += fill_cost;

	std::stringstream msg;
	msg << fill_qty << " for " << fill_cost;

	if (quantityOnMarket_>0)
		history_.emplace_back(PartialFilled, msg.str());
	else
		history_.emplace_back(Filled, msg.str());
	
	std::cerr << "onFilled ";
	sendStatusToSocket();
}

void
Order::onCancelRequested()
{
	history_.emplace_back(CancelRequested);
}

using namespace rapidjson;



void Order::sendStatusToSocket()
{

	int status = 99999;
	

	
	Order::StateChange state = currentState();


	/*
	"Pending", (0 = New)
	"In Process" (1 = Partially filled)
	"Completed" (2 = Filled)
	"Cancelled" (4 = Canceled)
	*/


	switch (state.state_)
	{
	case Order::Accepted:
		status = 0;
		break;
	case Order::PartialFilled:
		status = 1;
		break;
	case Order::Filled:
		status = 2;
		break;	
	case Order::Cancelled:
		status = 4;
		break;
	}
	
	/*
	StringBuffer s;
	Writer<StringBuffer> writer(s);
	writer.StartObject();

	writer.String("11");
	writer.String(ClOrdID_);

	writer.String("38");
	writer.Uint(quantityOnMarket_);

	writer.String("6");
	writer.Double(price_);

	writer.String("39");
	writer.Uint(status);

	/*
 int32_t quantityOnMarket_
  double price_;
  book::Quantity qty_;
  std::string ClOrdID_;
  std::string status_;
  std::vector<StateChange> history_;
  bool verbose_;
  */

	//
	signalOrderChanged(ClOrdID_, quantityOnMarket_, price_, status);
	/*
	writer.EndObject();
	std::string line = s.GetString();
	signalOrderChanged(line);
	*/
}

void
Order::onCancelled()
{
	quantityOnMarket_ = 0;
	history_.emplace_back(Cancelled);
	sendStatusToSocket();
	
}

void
Order::onCancelRejected(const char * reason)
{
	history_.emplace_back(CancelRejected, reason);

}

void
Order::onReplaceRequested(
const int32_t& size_delta,
liquibook::book::Price new_price)
{
	std::stringstream msg;
	if (size_delta != liquibook::book::SIZE_UNCHANGED)
	{
		msg << "Quantity change: " << size_delta << ' ';
	}
	if (new_price != liquibook::book::PRICE_UNCHANGED)
	{
		msg << "New Price " << new_price;
	}
	history_.emplace_back(ModifyRequested, msg.str());
}

void Order::onReplaced(const int32_t& size_delta,
liquibook::book::Price new_price)
{
	std::stringstream msg;
	if (size_delta != liquibook::book::SIZE_UNCHANGED)
	{
		qty_ += size_delta;
		quantityOnMarket_ += size_delta;
		msg << "Quantity change: " << size_delta << ' ';
	}
	if (new_price != liquibook::book::PRICE_UNCHANGED)
	{
		price_ = new_price;
		msg << "New Price " << new_price;
	}
	history_.emplace_back(Modified, msg.str());
}

void
Order::onReplaceRejected(const char * reason)
{
	history_.emplace_back(ModifyRejected, reason);
}

std::string Order::getStringState() //вызывается из publisher_main
{
	//return "lala";
	
	Order::StateChange state = currentState();
	
	

	switch (state.state_)
	{
	case Order::Submitted:
		return "Submitted";
		break;
	case Order::Rejected:
		return "Rejected";
		break;
	case Order::Accepted:
		return "Accepted";
		break;
	case Order::ModifyRequested:
		return "ModifyRequested";
		break;
	case Order::ModifyRejected:
		return "ModifyRejected";
		break;
	case Order::Modified:
		return "Modified";
		break;
	case Order::PartialFilled:
		return "PartialFilled";
		break;
	case Order::Filled:
		return "Filled";
		break;
	case Order::CancelRequested:
		return"CancelRequested";
		break;
	case Order::CancelRejected:
		return "CancelRejected";
		break;
	case Order::Cancelled:
		return "Cancelled";
		break;
	case Order::Unknown:
		return "Unknown";
		break;
	}
	//out << event.description_;


}

std::ostream & operator << (std::ostream & out, const Order::StateChange & event)
{
	out << "{";
	switch (event.state_)
	{
	case Order::Submitted:
		out << "Submitted";
		break;
	case Order::Rejected:
		out << "Rejected";
		break;
	case Order::Accepted:
		out << "Accepted";
		break;
	case Order::ModifyRequested:
		out << "ModifyRequested";
		break;
	case Order::ModifyRejected:
		out << "ModifyRejected";
		break;
	case Order::Modified:
		out << "Modified";
		break;
	case Order::PartialFilled:
		out << "PartialFilled ";
		break;
	case Order::Filled:
		out << "Filled ";
		break;
	case Order::CancelRequested:
		out << "CancelRequested ";
		break;
	case Order::CancelRejected:
		out << "CancelRejected ";
		break;
	case Order::Cancelled:
		out << "Cancelled ";
		break;
	case Order::Unknown:
		out << "Unknown ";
		break;
	}
	out << event.description_;
	out << "}";
	return out;
}

std::ostream & operator << (std::ostream & out, const Order & order)
{
	out << "[#" << order.order_ClOrdID();
	out << ' ' << (order.is_buy() ? "BUY" : "SELL");
	out << ' ' << order.order_qty();
	//out << ' ' << order.symbol();
	if (order.price() == 0)
	{
		out << " MKT";
	}
	else
	{
		out << " $" << order.price();
	}

	if (order.stop_price() != 0)
	{
		out << " STOP " << order.stop_price();
	}

	out << (order.all_or_none() ? " AON" : "")
		<< (order.immediate_or_cancel() ? " IOC" : "");

	auto onMarket = order.quantityOnMarket();
	if (onMarket != 0)
	{
		out << " Open: " << onMarket;
	}

	auto filled = order.quantityFilled();
	if (filled != 0)
	{
		out << " Filled: " << filled;
	}

	auto cost = order.fillCost();
	if (cost != 0)
	{
		out << " Cost: " << cost;
	}

	if (order.isVerbose())
	{
		const Order::History & history = order.history();
		for (auto event = history.begin(); event != history.end(); ++event)
		{
			out << "\n\t" << *event;
		}
	}
	else
	{
		out << " Last Event:" << order.currentState();
	}

	out << ']';

	return out;
}


uint32_t
Order::quantityOnMarket() const
{
	return quantityOnMarket_;
}

uint32_t
Order::quantityFilled() const
{
	return quantityFilled_;
}

uint32_t
Order::fillCost() const
{
	return fillCost_;
}


} 

} // End namespace

