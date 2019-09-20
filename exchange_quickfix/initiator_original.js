var df = require('dateformat');
var events = require('events');
var quickfix = require('./index');
var common = require('./common');
var path = require('path');
var initiator = quickfix.initiator;

var options = {
  credentials: {
    username: "USERNAME",
    password: "PASSWORD"
  },
  ssl : true,
  propertiesFile: path.join(__dirname, "initiator.properties")
};

// extend prototype
function inherits (target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}

inherits(initiator, events.EventEmitter);

var fixClient = new initiator(
{
  onCreate: function(sessionID) {
    fixClient.emit('onCreate', common.stats(fixClient, sessionID));
  },
  onLogon: function(sessionID) {
    fixClient.emit('onLogon', common.stats(fixClient, sessionID));
  },
  onLogout: function(sessionID) {
    fixClient.emit('onLogout', common.stats(fixClient, sessionID));
  },
  onLogonAttempt: function(message, sessionID) {
    fixClient.emit('onLogonAttempt', common.stats(fixClient, sessionID, message));
  },
  toAdmin: function(message, sessionID) {
    fixClient.emit('toAdmin', common.stats(fixClient, sessionID, message));
  },
  fromAdmin: function(message, sessionID) {
    fixClient.emit('fromAdmin', common.stats(fixClient, sessionID, message));
  },
  fromApp: function(message, sessionID) {
    fixClient.emit('fromApp', common.stats(fixClient, sessionID, message));
  }
}, options);


fixClient.start(function() {
    console.log("FIX Initiator Started");
  var order = {

//ADDING ORDER WITH ID

/*
 header: {
	8:  'FIX.4.4',
        35: 'D',
        49: "CLIENT1",
        56: "ORDERMATCH"
},
 tags: {
        //52:=20181016-15:07:03.103383
        11: 16546,
        21: 1,
        38: 1,
        40: 2,
        44: 63,
        54: 2,
        55: 'ETH',
        59: 1,
	60: df(new Date(), "yyyymmdd-HH:MM:ss.l")
	}
};

*/
/*
//cancel order by id
header: {
	8:  'FIX.4.4',
        35: 'F',
        49: "CLIENT1",
        56: "ORDERMATCH"
},
 tags: {
        11: 7777, //request id
        54: 2,
	41: 16546, //order id
	60: df(new Date(), "yyyymmdd-HH:MM:ss.l"),
	58: 'ETH' //symbol
}
};
*/


//getting status by id
header: {
	8:  'FIX.4.4',
        35: 'H',
        49: "CLIENT1",
        56: "ORDERMATCH"
},
 tags: {
        11: 16546, //order id
        54: 2, //side
	526: 'ETH' //symbol
}
};

  fixClient.send(order, function() {
    console.log("Order sent!");
    common.printStats(fixClient);
    process.stdin.resume();
  });
});
