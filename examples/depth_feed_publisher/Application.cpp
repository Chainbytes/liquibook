/****************************************************************************
** Copyright (c) 2001-2014
**
** This file is part of the QuickFIX FIX Engine
**
** This file may be distributed under the terms of the quickfixengine.org
** license as defined by quickfixengine.org and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.quickfixengine.org/LICENSE for licensing information.
**
** Contact ask@quickfixengine.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifdef _MSC_VER
#pragma warning( disable : 4503 4355 4786 )
#else
#include "config.h"
#endif

#include "Application.h"
#include "quickfix/Session.h"

#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix44/ExecutionReport.h"



void Application::onLogon( const FIX::SessionID& sessionID ) {}

void Application::onLogout( const FIX::SessionID& sessionID ) {}

void Application::fromApp( const FIX::Message& message,
                           const FIX::SessionID& sessionID )
throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
{
	//вызывается когда получено входящие сообщение от трейдер-клиента
	std::cerr << "0#####################################################";

  crack( message, sessionID );
}

void Application::onMessage(const FIX44::OrderStatusRequest& message,
	const FIX::SessionID& sessionID)
{
	std::string string_ClOrdID = message.getField(11);
	std::string symbol_str = message.getField(526);
	

	

	std::cerr << "\nSTATUS REQUESTED FOR ORDER_______ClOrdID=" << string_ClOrdID << " Symbol: " << symbol_str << "\n___________";

	if (symbol_str == "undefined")
		return;

	Order_struct order = *signalGetStatusOrder(symbol_str, string_ClOrdID);

	std::cout << "\n_______\n" << "Status for order#" << string_ClOrdID << "\nQTY=" << order.qty << "\nprice=" << order.price << "\nis_buy=" << order.is_buy << "\nstatus=" << order.status;

	
	FIX::Symbol symbol = symbol_str;
	
	//FIX::OrdType ordType;
	FIX::OrderQty orderQty = order.qty;
	FIX::Price price = order.price;
	FIX::ClOrdID clOrdID = string_ClOrdID;
	FIX::Account account;
	FIX::OrdStatus ordStatus;
	
	if (order.status == "Cancelled")
		ordStatus = FIX::OrdStatus_CANCELED;

	if (order.status == "Filled")
		ordStatus = FIX::OrdStatus_FILLED;

	if (order.status == "PartialFilled")
		ordStatus = FIX::OrdStatus_PARTIALLY_FILLED;

	//if (order.status.compare(std::string("Accepted"))==0)
	if (order.status == "Accepted")
		{
			ordStatus = FIX::OrdStatus_NEW;
			std::cout << "\n@@@@@@@@@_\n";
		}

	/*
		"Pending", (0 = New)
		"In Process" (1 = Partially filled)
		"Completed" (2 = Filled)
		"Cancelled" (4 = Canceled)
	*/

	FIX::Side side;

	if (order.is_buy)
		side = FIX::Side(FIX::Side_BUY);
	else
		side = FIX::Side(FIX::Side_SELL);



	FIX44::ExecutionReport executionReport = FIX44::ExecutionReport
		//(FIX::OrderID(genOrderID()),
		//FIX::ExecID(genExecID()),
		(FIX::OrderID(clOrdID),
		FIX::ExecID("2"),
		FIX::ExecType(FIX::ExecType_TRADE),
		FIX::OrdStatus(ordStatus),
		side,
		FIX::LeavesQty(0),
		FIX::CumQty(orderQty),
		FIX::AvgPx(price));



	executionReport.set(clOrdID);
	executionReport.set(symbol);
	executionReport.set(orderQty);
	executionReport.set(FIX::LastQty(orderQty));
	executionReport.set(FIX::LastPx(price));




	/*
	if (message.isSet(account))
		executionReport.setField(message.get(account));
		*/
	FIX::Session::sendToTarget(executionReport, sessionID);





	/*
	std::string ClOrdID;
	int qty;
	double price;
	bool is_buy;
	std::string status;
	*/

	

	//вовзвращаем мап: qty, price, side, status в мапе
	//std::cout << "\nStatus for order#" << symbol <<" : " << *signalGetStatusOrder(symbol, string_ClOrdID);
	

}


void Application::onMessage(const FIX44::OrderCancelRequest& message, const FIX::SessionID& sessionID)
{
	std::string string_ClOrdID = message.getField(41);
	std::string symbol = message.getField(58);


	std::cerr << "\nCANCEL REQUESTED FOR ORDER_______ClOrdID=" << string_ClOrdID << " Symbol: "<<symbol<<"\n___________";

	std::cout << "\norder canceling result:\n" << *signalCancelOrder(symbol, string_ClOrdID);
	



}

void Application::onMessage(const FIX44::NewOrderSingle& message,
	const FIX::SessionID& sessionID)
{
	std::cerr << "4444444################################################\n";
	//будем использовать это
	FIX::Symbol symbol;
	FIX::Side side;
	FIX::OrdType ordType;
	FIX::OrderQty orderQty;
	FIX::Price price;
	FIX::ClOrdID clOrdID;
	FIX::Account account;

	

	message.get(ordType);

	if (ordType != FIX::OrdType_LIMIT)
		throw FIX::IncorrectTagValue(ordType.getField());

	message.get(symbol);
	message.get(side);
	message.get(orderQty);
	message.get(price);
	message.get(clOrdID);


	std::string string_ClOrdID;
	string_ClOrdID = message.getField(11);

	std::cerr << "_______\nClOrdID=" << string_ClOrdID << "\n___________";
	
	/*
	Order order( clOrdID, symbol, senderCompID, targetCompID,
	convert( side ), convert( ordType ),
	price, (long)orderQty );

	processOrder( order );
	*/

	//buy/sell
	

	//price
	//	double price = order.getPrice();

	//qty
	//	long qty = order.getQuantity();

	//symbol
	//const std::string &symbol = order.getSymbol();


	//, double, int, std::string) > 
	

	

	FIX44::ExecutionReport executionReport;
	
	
	if (message.isSet(account))
		executionReport.setField(message.get(account));
		
	try
	{
		bool buyBool = false;
		
		
		if (side == FIX::Side(FIX::Side_BUY))
			buyBool = true;


		std::string add_order_result = *signalProcessOrder(buyBool, price, orderQty, symbol, string_ClOrdID, "");
			std::cout << "\norder adding result:" << add_order_result <<"\n";

			if (add_order_result == "true") //отправляем репорт, что удачно
			{
				executionReport = FIX44::ExecutionReport
					//(FIX::OrderID(genOrderID()),
					//FIX::ExecID(genExecID()),
					(FIX::OrderID("1"),
					FIX::ExecID("2"),
					FIX::ExecType(FIX::ExecType_TRADE),
					FIX::OrdStatus(FIX::OrdStatus_NEW),
					side,
					FIX::LeavesQty(0),
					FIX::CumQty(orderQty),
					FIX::AvgPx(price));

				executionReport.set(clOrdID);
				executionReport.set(symbol);
				executionReport.set(orderQty);
				executionReport.set(FIX::LastQty(orderQty));
				executionReport.set(FIX::LastPx(price));
			}
			else
			{
				executionReport = FIX44::ExecutionReport
					//(FIX::OrderID(genOrderID()),
					//FIX::ExecID(genExecID()),
					(FIX::OrderID("1"),
					FIX::ExecID("2"),
					FIX::ExecType(FIX::ExecType_TRADE),
					FIX::OrdStatus(FIX::OrdStatus_PENDING_CANCEL),
					side,
					FIX::LeavesQty(0),
					FIX::CumQty(orderQty),
					FIX::AvgPx(price));
				
				executionReport.set(clOrdID);
				executionReport.set(symbol);
				executionReport.set(orderQty);
				executionReport.set(FIX::LastQty(orderQty));
				executionReport.set(FIX::LastPx(price));
				
				executionReport.setField(FIX::Text("Order with clOrdID=" + string_ClOrdID + " already added"));
			}
		
		
		FIX::Session::sendToTarget(executionReport, sessionID); //отправляем репорт
		
	}
	catch (FIX::SessionNotFound&) {}
}

void Application::onMessage( const FIX42::NewOrderSingle& message, const FIX::SessionID& )
{

	std::cerr << "1#####################################################";

  FIX::SenderCompID senderCompID;
  FIX::TargetCompID targetCompID;
  FIX::ClOrdID clOrdID;
  FIX::Symbol symbol;
  FIX::Side side;
  FIX::OrdType ordType;
  FIX::Price price;
  FIX::OrderQty orderQty;
  FIX::TimeInForce timeInForce( FIX::TimeInForce_DAY );

  message.getHeader().get( senderCompID );
  message.getHeader().get( targetCompID );
  message.get( clOrdID );
  message.get( symbol );
  message.get( side );
  message.get( ordType );
  if ( ordType == FIX::OrdType_LIMIT )
    message.get( price );
  message.get( orderQty );
  message.getFieldIfSet( timeInForce );

  try
  {
    if ( timeInForce != FIX::TimeInForce_DAY )
      throw std::logic_error( "Unsupported TIF, use Day" );

    Order order( clOrdID, symbol, senderCompID, targetCompID,
                 convert( side ), convert( ordType ),
                 price, (long)orderQty );

    processOrder( order );
  }
  catch ( std::exception & e )
  {
    rejectOrder( senderCompID, targetCompID, clOrdID, symbol, side, e.what() );
  }
}

void Application::onMessage( const FIX42::OrderCancelRequest& message, const FIX::SessionID& )
{

	std::cerr << "2#####################################################";
  FIX::OrigClOrdID origClOrdID;
  FIX::Symbol symbol;
  FIX::Side side;

  message.get( origClOrdID );
  message.get( symbol );
  message.get( side );

  try
  {
    processCancel( origClOrdID, symbol, convert( side ) );
  }
  catch ( std::exception& ) {}}

void Application::onMessage( const FIX42::MarketDataRequest& message, const FIX::SessionID& )
{
	std::cerr << "3#####################################################";
  FIX::MDReqID mdReqID;
  FIX::SubscriptionRequestType subscriptionRequestType;
  FIX::MarketDepth marketDepth;
  FIX::NoRelatedSym noRelatedSym;
  FIX42::MarketDataRequest::NoRelatedSym noRelatedSymGroup;

  message.get( mdReqID );
  message.get( subscriptionRequestType );
  if ( subscriptionRequestType != FIX::SubscriptionRequestType_SNAPSHOT )
    throw( FIX::IncorrectTagValue( subscriptionRequestType.getField() ) );
  message.get( marketDepth );
  message.get( noRelatedSym );

  for ( int i = 1; i <= noRelatedSym; ++i )
  {
    FIX::Symbol symbol;
    message.getGroup( i, noRelatedSymGroup );
    noRelatedSymGroup.get( symbol );
  }
}

void Application::onMessage( const FIX43::MarketDataRequest& message, const FIX::SessionID& )
{

	std::cerr << "4#####################################################";

    std::cout << message.toXML() << std::endl;
}

void Application::updateOrder( const Order& order, char status )
{
	std::cerr << "5#####################################################";
  FIX::TargetCompID targetCompID( order.getOwner() );
  FIX::SenderCompID senderCompID( order.getTarget() );

  FIX42::ExecutionReport fixOrder
  ( FIX::OrderID ( order.getClientID() ),
    FIX::ExecID ( m_generator.genExecutionID() ),
    FIX::ExecTransType ( FIX::ExecTransType_NEW ),
    FIX::ExecType ( status ),
    FIX::OrdStatus ( status ),
    FIX::Symbol ( order.getSymbol() ),
    FIX::Side ( convert( order.getSide() ) ),
    FIX::LeavesQty ( order.getOpenQuantity() ),
    FIX::CumQty ( order.getExecutedQuantity() ),
    FIX::AvgPx ( order.getAvgExecutedPrice() ) );

  fixOrder.set( FIX::ClOrdID( order.getClientID() ) );
  fixOrder.set( FIX::OrderQty( order.getQuantity() ) );

  if ( status == FIX::OrdStatus_FILLED ||
       status == FIX::OrdStatus_PARTIALLY_FILLED )
  {
    fixOrder.set( FIX::LastShares( order.getLastExecutedQuantity() ) );
    fixOrder.set( FIX::LastPx( order.getLastExecutedPrice() ) );
  }

  try
  {
    FIX::Session::sendToTarget( fixOrder, senderCompID, targetCompID );
  }
  catch ( FIX::SessionNotFound& ) {}}

void Application::rejectOrder
( const FIX::SenderCompID& sender, const FIX::TargetCompID& target,
  const FIX::ClOrdID& clOrdID, const FIX::Symbol& symbol,
  const FIX::Side& side, const std::string& message )
{

	std::cerr << "5.1#####################################################";
  FIX::TargetCompID targetCompID( sender.getValue() );
  FIX::SenderCompID senderCompID( target.getValue() );

  FIX42::ExecutionReport fixOrder
  ( FIX::OrderID ( clOrdID.getValue() ),
    FIX::ExecID ( m_generator.genExecutionID() ),
    FIX::ExecTransType ( FIX::ExecTransType_NEW ),
    FIX::ExecType ( FIX::ExecType_REJECTED ),
    FIX::OrdStatus ( FIX::ExecType_REJECTED ),
    symbol, side, FIX::LeavesQty( 0 ), FIX::CumQty( 0 ), FIX::AvgPx( 0 ) );

  fixOrder.set( clOrdID );
  fixOrder.set( FIX::Text( message ) );

  try
  {
    FIX::Session::sendToTarget( fixOrder, senderCompID, targetCompID );
  }
  catch ( FIX::SessionNotFound& ) {}}

void Application::processOrder( const Order& order )
{
	//SelectCell(10000, 10000);

	std::cerr << "6#####################################################";
	/*
  if ( m_orderMatcher.insert( order ) )
  {
    acceptOrder( order );

    std::queue < Order > orders;
    m_orderMatcher.match( order.getSymbol(), orders );

    while ( orders.size() )
    {
      fillOrder( orders.front() );
      orders.pop();
    }

	//buy/sell
	bool buy=false;
	FIX::Side side = order.getSide();
	if (side == buy)
		buy = true;
	
	//price
	double price = order.getPrice();

	//qty
	long qty = order.getQuantity();

		//symbol
	const std::string &symbol = order.getSymbol();


	//, double, int, std::string) > 
	signalProcessOrder(buy,price,qty,symbol, symbol);
  }
  else
    rejectOrder( order );
	*/
}

void Application::processCancel( const std::string& id,
                                 const std::string& symbol, Order::Side side )
{
  Order & order = m_orderMatcher.find( symbol, side, id );
  order.cancel();
  cancelOrder( order );
  m_orderMatcher.erase( order );
}

Order::Side Application::convert( const FIX::Side& side )
{
  switch ( side )
  {
    case FIX::Side_BUY: return Order::buy;
    case FIX::Side_SELL: return Order::sell;
    default: throw std::logic_error( "Unsupported Side, use buy or sell" );
  }
}

Order::Type Application::convert( const FIX::OrdType& ordType )
{
  switch ( ordType )
  {
    case FIX::OrdType_LIMIT: return Order::limit;
    default: throw std::logic_error( "Unsupported Order Type, use limit" );
  }
}

FIX::Side Application::convert( Order::Side side )
{
  switch ( side )
  {
    case Order::buy: return FIX::Side( FIX::Side_BUY );
    case Order::sell: return FIX::Side( FIX::Side_SELL );
    default: throw std::logic_error( "Unsupported Side, use buy or sell" );
  }
}

FIX::OrdType Application::convert( Order::Type type )
{
  switch ( type )
  {
    case Order::limit: return FIX::OrdType( FIX::OrdType_LIMIT );
    default: throw std::logic_error( "Unsupported Order Type, use limit" );
  }
}
