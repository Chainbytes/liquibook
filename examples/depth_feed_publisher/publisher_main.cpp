#include "exchange.h"
#include "depth_feed_publisher.h"
#include "depth_feed_connection.h"
#include "order.h"

#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"
#include "quickfix/SocketAcceptor.h"
#include "quickfix/Session.h"
#include "quickfix/SessionSettings.h"
#include "Application.h"
#include "quickfix/Exceptions.h"
#include "config.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "quickfix/fix42/ExecutionReport.h"

#include "mysql_connection.h"
#include "mysql_driver.h"


#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>



#include "sio_client.cpp"
#include "sio_client_impl.cpp"
#include "sio_packet.cpp"
#include "sio_socket.cpp"

#include "sio_client.h"
#include "sio_message.h"
#include "sio_socket.h"
#include "sio_packet.h"


#if __STDC_VERSION__ < 199901L
#  if __GNUC__ >= 2
#    define EXAMPLE_FUNCTION __FUNCTION__
#  else
#    define EXAMPLE_FUNCTION "(function n/a)"
#  endif
#elif defined(_MSC_VER)
#  if _MSC_VER < 1300
#    define EXAMPLE_FUNCTION "(function n/a)"
#  else
#    define EXAMPLE_FUNCTION __FUNCTION__
#  endif
#elif (defined __func__)
#  define EXAMPLE_FUNCTION __func__
#else
#  define EXAMPLE_FUNCTION "(function n/a)"
#endif

using namespace liquibook;

struct SecurityInfo {
  std::string symbol;
  double ref_price;
  SecurityInfo(const char* sym, double price)
  : symbol(sym),
    ref_price(price)
  {
  }
};


typedef std::vector<SecurityInfo> SecurityVector;

void create_securities(SecurityVector& securities);
void populate_exchange(examples::Exchange& exchange, 
                       const SecurityVector& securities);
void generate_orders(examples::Exchange& exchange, 
                     const SecurityVector& securities);


examples::Exchange exchange1(0, 0);
SecurityVector securities1;
sio::client orderStatusSocket;

void OnPlayerSelectCell(int x, int y)
{
	std::cout << "Player selected cell: " << x << ", " << y << std::endl;
}

std::string cancelOrder(std::string symbol, std::string ClOrdID)
{
	return exchange1.cancel_order_by_ClOrdID(symbol, ClOrdID);
}

Application::Order_struct getStatusOrder(std::string symbol, std::string ClOrdID)
{
	std::cout << "std::string getStatusOrder";
	return exchange1.getStatusOrder(symbol, ClOrdID);
}


void sendJSONtoSocket(std::string str)
{
	orderStatusSocket.socket()->emit("order_status",str);
}


std::string processOrder(bool is_buy, double price, double qty, std::string symbol, std::string ClOrdID, std::string status)
{
	//status нужен, если ордер восстанавливается из бд

	std::cout << "\n\nORDER: " << is_buy << ", " << price << ", " << qty << ", " << symbol <<", " << ClOrdID << std::endl;


	
	std::cout << "__________";


	/*SecurityInfo info;
	info.symbol = symbol;
	info.
	*/

	//std::string str;
	const char * c = symbol.c_str();

	 const SecurityInfo& sec = SecurityInfo (c, double(0));

	bool found = false;
	for (int i = 0; i < securities1.size(); i++)
	{
		// which security

		const SecurityInfo& sec1 = securities1[i];

		if (sec.symbol == sec1.symbol)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		std::cout << "!Symbol did not found";
		return "!found";
	}
	

	std::cout << "__________";


	book::Quantity qty1 = qty;

	// order
	examples::OrderPtr order(new examples::Order(is_buy, price, qty1, ClOrdID, status));

	order->signalOrderChanged.connect(&sendJSONtoSocket);

	// add order
	if (exchange1.add_order(sec.symbol, order))
		return "true";
	else
		return "false";
	


	//const SecurityInfo& sec = securities1[1];

//
	//ищем символ
	/*
	// which security
	size_t index = std::rand() % num_securities;
	const SecurityInfo& sec = securities[index];
	// side
	bool is_buy = (std::rand() % 2) != 0;
	// price
	uint32_t price_base = uint32_t(sec.ref_price * 100);
	uint32_t delta_range = price_base / 50;  // +/- 2% of base
	int32_t delta = std::rand() % delta_range;
	delta -= (delta_range / 2);
	double price = double(price_base + delta) / 100;

	// qty
	book::Quantity qty = (std::rand() % 10 + 1) * 100;

	// order
	examples::OrderPtr order(new examples::Order(is_buy, price, qty));

	// add order
	exchange.add_order(sec.symbol, order);

*/
}


int main(int argc, const char* argv[])
{

  try
  {
	
    // Feed connection
    examples::DepthFeedConnection connection(argc, argv);

    // Open connection in background thread
    connection.accept();
    boost::function<void ()> acceptor1(
        boost::bind(&examples::DepthFeedConnection::run, &connection));
    boost::thread acceptor_thread(acceptor1);

	//open socket for sending order status==========

	//with htread=========
	
	
	orderStatusSocket.connect("http://127.0.0.1:3005");
		

  //===================================================================
    // Create feed publisher
    examples::DepthFeedPublisher feed;
    feed.set_connection(&connection);

	exchange1 = examples::Exchange(&feed, &feed);
 

    // Create securities
    SecurityVector securities;
    create_securities(securities);

	/////

	securities1.push_back(SecurityInfo("ETH",0));
	securities1.push_back(SecurityInfo("LTC", 0));
	securities1.push_back(SecurityInfo("DASH", 0));
	securities1.push_back(SecurityInfo("DOGE", 0));

	/////

    // Populate exchange with securities
    populate_exchange(exchange1, securities1);
  
    //=========== load orders from db=============
	
	try {

		sql::mysql::MySQL_Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = sql::mysql::get_driver_instance();
		con = driver->connect("tcp://worldex.io", "root", "WorldexDB_Exchange@141%%");
		/* Connect to the MySQL test database */
		con->setSchema("chainbyte_exchange");

		stmt = con->createStatement();
		res = stmt->executeQuery("SELECT unique_order_id, coin_code, order_type, market_amount, price, status  FROM `zozocoinex_orders` WHERE status != 'Completed' and status != 'Cancelled' and base_coin_code = 'BTC' and coin_code = 'ETH' ORDER BY order_place_date");
		while (res->next()) {
			cout << "\t...Order from DB:\n";
			bool is_buy=false;
			double price=0;
			double qty=0;
			std::string symbol;
			std::string clOrdID;
			std::string status;

			if (res->getString("order_type") == "Buy")
				is_buy = true;

			clOrdID = res->getString("unique_order_id");
			symbol = res->getString("coin_code");
			qty = res->getDouble("market_amount");
			price = res->getDouble("price");
			status = res->getString("status");

			/* Access column data by alias or column name */
			cout << "clOrdID = " << clOrdID << endl;
			cout << "symbol = " << symbol << endl;
			cout << "order_type = " << res->getString("order_type") << endl;
			cout << "quantity = " << qty << endl;
			cout << "price = " << price << endl;
			cout << "status = " << status << endl;
			cout << "_________________________________\n";
			cout << "Order clOrdID = " << clOrdID << " adding result: " << processOrder(is_buy, price, qty, symbol, clOrdID, status);
		}
		delete res;
		delete stmt;
		delete con;

	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << EXAMPLE_FUNCTION << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		
	}



	
	//===========

	std::string file("ordermatch.cfg");

	FIX::SessionSettings settings(file);

	Application application;

	application.SelectCell.connect(&OnPlayerSelectCell);

	application.signalProcessOrder.connect(&processOrder);

	application.signalCancelOrder.connect(&cancelOrder);

	application.signalGetStatusOrder.connect(&getStatusOrder);

	FIX::FileStoreFactory storeFactory(settings);
	FIX::ScreenLogFactory logFactory(settings);
	FIX::SocketAcceptor acceptor(application, storeFactory, settings, logFactory);
	std::cerr << "_________-2_____";
	acceptor.start();
	std::cerr << "_________-1_____";


//	MyClass *myClass = new MyClass;
//	application.myClass = myClass;
//	sig.connect(application);
//	sig.connect(World());


	while (true)
	{
		std::string value;
		std::cin >> value;

		std::cerr << "_________0_____";

		if (value == "#symbols")
		{
			application.orderMatcher().display();
			std::cerr << "_________0_____";
		}
		else if (value == "#quit")
			break;
		else
		{
			application.orderMatcher().display(value);
			std::cerr << "_________1_____"<< value;
		}

		std::cout << std::endl;
	}
	acceptor.stop();
	return 0;


	//=============
  }
  catch (const std::exception & ex)
  {
    std::cerr << "Exception caught at main level: " << ex.what() << std::endl;
    return -1;
  }

  std::cerr << "##############################################################################################################";
  std::string file = argv[1];
   
  try
  {
	  FIX::SessionSettings settings(file);

	  Application application;
	  FIX::FileStoreFactory storeFactory(settings);
	  FIX::ScreenLogFactory logFactory(settings);
	  FIX::SocketAcceptor acceptor(application, storeFactory, settings, logFactory);

	  acceptor.start();
	  while (true)
	  {
		  std::string value;
		  std::cin >> value;

		  if (value == "#symbols")
			  application.orderMatcher().display();
		  else if (value == "#quit")
			  break;
		  else
			  application.orderMatcher().display(value);

		  std::cout << std::endl;
	  }
	  acceptor.stop();
	  return 0;
  }
  catch (std::exception & e)
  {
	  std::cout << e.what() << std::endl;
	  return 1;
  };
  
  return 0;
}

void
create_securities(SecurityVector& securities) {
  securities.push_back(SecurityInfo("ETH", 436.36));
  securities.push_back(SecurityInfo("LTC", 45.06));
  securities.push_back(SecurityInfo("DASH", 43.93));
  securities.push_back(SecurityInfo("DOGE", 67.09));
  securities.push_back(SecurityInfo("ADSK", 38.34));
  securities.push_back(SecurityInfo("AKAM", 43.65));
  securities.push_back(SecurityInfo("ALTR", 31.90));
  securities.push_back(SecurityInfo("ALXN", 96.28));
  securities.push_back(SecurityInfo("AMAT", 14.623));
  securities.push_back(SecurityInfo("AMGN", 104.88));
  securities.push_back(SecurityInfo("AMZN", 247.74));
  securities.push_back(SecurityInfo("ATVI", 14.69));
  securities.push_back(SecurityInfo("AVGO", 31.38));
  securities.push_back(SecurityInfo("BBBY", 68.81));
  securities.push_back(SecurityInfo("BIDU", 85.09));
  securities.push_back(SecurityInfo("BIIB", 214.89));
  securities.push_back(SecurityInfo("BMC", 45.325));
  securities.push_back(SecurityInfo("BRCM", 35.60));
  securities.push_back(SecurityInfo("CA", 26.97));
  securities.push_back(SecurityInfo("CELG", 116.901));
  securities.push_back(SecurityInfo("CERN", 95.24));
  securities.push_back(SecurityInfo("CHKP", 46.43));
  securities.push_back(SecurityInfo("CHRW", 58.89));
  securities.push_back(SecurityInfo("CMCSA", 41.99));
  securities.push_back(SecurityInfo("COST", 108.16));
  securities.push_back(SecurityInfo("CSCO", 20.425));
  securities.push_back(SecurityInfo("CTRX", 57.419));
  securities.push_back(SecurityInfo("CTSH", 63.62));
  securities.push_back(SecurityInfo("CTXS", 62.38));
  securities.push_back(SecurityInfo("DELL", 13.33));
  securities.push_back(SecurityInfo("DISCA", 78.18));
  securities.push_back(SecurityInfo("DLTR", 47.91));
  securities.push_back(SecurityInfo("DTV", 56.56));
  securities.push_back(SecurityInfo("EBAY", 52.215));
  securities.push_back(SecurityInfo("EQIX", 217.015));
  securities.push_back(SecurityInfo("ESRX", 59.26));
  securities.push_back(SecurityInfo("EXPD", 35.03));
  securities.push_back(SecurityInfo("EXPE", 55.15));
  securities.push_back(SecurityInfo("FAST", 48.13));
  securities.push_back(SecurityInfo("FB", 27.52));
  securities.push_back(SecurityInfo("FFIV", 74.11));
  securities.push_back(SecurityInfo("FISV", 87.58));
  securities.push_back(SecurityInfo("FOSL", 95.09));
  securities.push_back(SecurityInfo("GILD", 50.06));
  securities.push_back(SecurityInfo("GOLD", 78.681));
  securities.push_back(SecurityInfo("GOOG", 817.08));
  securities.push_back(SecurityInfo("GRMN", 33.33));
  securities.push_back(SecurityInfo("HSIC", 89.44));
  securities.push_back(SecurityInfo("INTC", 23.9673));
  securities.push_back(SecurityInfo("INTU", 60.15));
  securities.push_back(SecurityInfo("ISRG", 492.3358));
  securities.push_back(SecurityInfo("KLAC", 53.83));
  securities.push_back(SecurityInfo("KRFT", 50.9001));
  securities.push_back(SecurityInfo("LBTYA", 73.99));
  securities.push_back(SecurityInfo("LIFE", 73.59));
  securities.push_back(SecurityInfo("LINTA", 21.44));
  securities.push_back(SecurityInfo("LLTC", 36.25));
  securities.push_back(SecurityInfo("MAT", 44.99));
  securities.push_back(SecurityInfo("MCHP", 36.1877));
  securities.push_back(SecurityInfo("MDLZ", 31.58));
  securities.push_back(SecurityInfo("MNST", 55.75));
  securities.push_back(SecurityInfo("MSFT", 32.75));
  securities.push_back(SecurityInfo("MU", 9.19));
  securities.push_back(SecurityInfo("MXIM", 30.59));
  securities.push_back(SecurityInfo("MYL", 28.90));
  securities.push_back(SecurityInfo("NTAP", 34.17));
  securities.push_back(SecurityInfo("NUAN", 18.89));
  securities.push_back(SecurityInfo("NVDA", 13.7761));
  securities.push_back(SecurityInfo("NWSA", 31.12));
  securities.push_back(SecurityInfo("ORCL", 33.19));
  securities.push_back(SecurityInfo("ORLY", 107.58));
  securities.push_back(SecurityInfo("PAYX", 36.32));
  securities.push_back(SecurityInfo("PCAR", 49.52));
  securities.push_back(SecurityInfo("PCLN", 697.62));
  securities.push_back(SecurityInfo("PRGO", 119.00));
  securities.push_back(SecurityInfo("QCOM", 61.925));
  securities.push_back(SecurityInfo("REGN", 242.49));
  securities.push_back(SecurityInfo("ROST", 65.20));
  securities.push_back(SecurityInfo("SBAC", 78.76));
  securities.push_back(SecurityInfo("SBUX", 60.07));
  securities.push_back(SecurityInfo("SHLD", 49.989));
  securities.push_back(SecurityInfo("SIAL", 77.95));
  securities.push_back(SecurityInfo("SIRI", 3.36));
  securities.push_back(SecurityInfo("SNDK", 51.23));
  securities.push_back(SecurityInfo("SPLS", 13.07));
  securities.push_back(SecurityInfo("SRCL", 108.15));
  securities.push_back(SecurityInfo("STX", 36.82));
  securities.push_back(SecurityInfo("SYMC", 24.325));
  securities.push_back(SecurityInfo("TXN", 36.28));
  securities.push_back(SecurityInfo("VIAB", 66.295));
  securities.push_back(SecurityInfo("VMED", 49.56));
  securities.push_back(SecurityInfo("VOD", 30.49));
  securities.push_back(SecurityInfo("VRSK", 61.1728));
  securities.push_back(SecurityInfo("VRTX", 77.255));
  securities.push_back(SecurityInfo("WDC", 54.76));
  securities.push_back(SecurityInfo("WFM", 89.35));
  securities.push_back(SecurityInfo("WYNN", 136.33));
  securities.push_back(SecurityInfo("XLNX", 37.59));
  securities.push_back(SecurityInfo("XRAY", 42.26));
  securities.push_back(SecurityInfo("YHOO", 24.32));
}

void
populate_exchange(examples::Exchange& exchange, const SecurityVector& securities) {
  SecurityVector::const_iterator sec;
  for (sec = securities.begin(); sec != securities.end(); ++sec) {
    exchange.add_order_book(sec->symbol);
  }
}

void
generate_orders(examples::Exchange& exchange, const SecurityVector& securities) {
  /*time_t now;
  time(&now);
  std::srand(uint32_t(now));

  size_t num_securities = securities.size();
  while (true) 
  {
    // which security
    size_t index = std::rand() % num_securities;
    const SecurityInfo& sec = securities[index];
    // side
    bool is_buy = (std::rand() % 2) != 0;
    // price
    uint32_t price_base = uint32_t(sec.ref_price * 100);
    uint32_t delta_range = price_base / 50;  // +/- 2% of base
    int32_t delta = std::rand() % delta_range;
    delta -= (delta_range / 2);
    double price = double (price_base + delta) / 100;

    // qty
    book::Quantity qty = (std::rand() % 10 + 1) * 100;

    // order
    examples::OrderPtr order(new examples::Order(is_buy, price, qty));

    // add order
    exchange.add_order(sec.symbol, order);

    // Wait for eyes to read
    sleep(1);
  }
  */
}
