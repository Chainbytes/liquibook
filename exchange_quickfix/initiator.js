const express = require("express");
const app = express();
const request = require("request");
const bodyParser = require('body-parser');

const io = require('socket.io-client');

app.use(bodyParser.json());
app.use(express.json());

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
    ssl: true,
    propertiesFile: path.join(__dirname, "initiator.properties")
};
// extend prototype
function inherits(target, source) {
    for (var k in source.prototype)
        target.prototype[k] = source.prototype[k];
}

inherits(initiator, events.EventEmitter);
var fixClient = new initiator(
        {
            onCreate: function (sessionID) {
                console.log('ok1');
                fixClient.emit('onCreate', common.stats(fixClient, sessionID));
            },
            onLogon: function (sessionID) {
                console.log('ok2');
                fixClient.emit('onLogon', common.stats(fixClient, sessionID));
            },
            onLogout: function (sessionID) {
                console.log('ok3');
                fixClient.emit('onLogout', common.stats(fixClient, sessionID));
            },
            onLogonAttempt: function (message, sessionID) {
                console.log('ok4');
                fixClient.emit('onLogonAttempt', common.stats(fixClient, sessionID, message));
            },
            toAdmin: function (message, sessionID) {
                console.log('ok5');
                fixClient.emit('toAdmin', common.stats(fixClient, sessionID, message));
            },
            fromAdmin: function (message, sessionID) {
                console.log('ok6');
                fixClient.emit('fromAdmin', common.stats(fixClient, sessionID, message));
            },
            fromApp: function (message, sessionID) {
                /*const socket = io('http://178.128.1.254:3005');
                socket.emit('order_status',message);
                console.log('socket emit');*/
                fixClient.emit('fromApp', common.stats(fixClient, sessionID, message));
                
            }
        }, options);


fixClient.start(function () {
        console.log("FIX Initiator Started");
    });
app.post('/place_order', function (req, res) {
    /*console.log(req.body.order_id);
     console.log(req.body.order_qty);
     console.log(req.body.order_type);
     console.log(req.body.order_price);
     console.log(req.body.order_symbol);
     console.log(req.body.order_allow_time);*/


    if (req.body.order_id == '' || req.body.order_qty == '' || req.body.order_type == '' || req.body.order_price == '' || req.body.order_symbol == "") {
        res.setHeader('Content-Type', 'application/json');
        res.send(JSON.stringify({status: false, error: 'No enough parameters.'}));
        return;
    }

    let order_id = req.body.order_id;
    let order_qty = req.body.order_qty;
    let order_type = req.body.order_type;
    let order_price = req.body.order_price;
    let order_symbol = req.body.order_symbol;
    let order_allow_time = req.body.order_allow_time;
    let currency = req.body.currency;
    let order_side = req.body.order_side;


 console.log(order_side);

var order;

if(order_price==-1) //cancel
{

/*
//cancel order by id
header: {
<------>8:  'FIX.4.4',
        35: 'F',
        49: "CLIENT1",
        56: "ORDERMATCH"
},
 tags: {
        11: 7777, //request id
        54: 2,
<------>41: 16546, //order id
<------>60: df(new Date(), "yyyymmdd-HH:MM:ss.l"),
<------>58: 'ETH' //symbol
}
};
*/
     order = {
        header: {
            8: 'FIX.4.4',
            35: 'F',
            49: "CLIENT1",
            56: "ORDERMATCH"
        },

        tags: {
            11: order_id,
	    41: order_id,
            54: order_side, //1=buy,2=sell
	    58: 'ETH',
            60: df(new Date(), "yyyymmdd-HH:MM:ss.l")
        }
    };
}
else {

	 order = {
        header: {
            8: 'FIX.4.4',
            35: 'D',
            49: "CLIENT1",
            56: "ORDERMATCH"
        },

        tags: {
            //52:=20181016-15:07:03.103383
            11: order_id,
            21: 1,
            38: order_qty,
            40: order_type, //1=Market,2=Limit,etc.
            44: order_price,
            54: order_side, //1=buy,2=sell
            55: order_symbol,
            59: order_allow_time,
            60: df(new Date(), "yyyymmdd-HH:MM:ss.l")
        }
    };
}
    console.log(order);
    
        fixClient.send(order, function () {
            console.log("Order sent!");
            common.printStats(fixClient);
            process.stdin.resume();
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify({status: true, message: 'Order successfully placed for process.'}));
            return;
        });
    

});
app.post('/get_order_data', function (req, res) {
    if (req.body.order_id == "" || req.body.order_side == "" || req.body.order_symbol == "") {
        res.setHeader('Content-Type', 'application/json');
        res.send(JSON.stringify({status: false, error: 'No enough parameters.'}));
        return;
    }
    let order_id = req.body.order_id;
    let order_side = req.body.order_side;
    let order_symbol = req.body.order_symbol;
    var order = {
        header: {
            8: 'FIX.4.4',
            35: 'H',
            49: "CLIENT1",
            56: "ORDERMATCH"
        },
        tags: {
            11: order_id, //order id
            54: order_side, //side
            526: order_symbol //symbol
        }

    };
    
        fixClient.send(order, function () {
            console.log("Order data get!");
            
            //common.printStats(fixClient);
            process.stdin.resume();
            res.setHeader('Content-Type', 'application/json');
            res.send(JSON.stringify({status: true, message: 'Order data get'}));
            return;
        });
    
});
app.listen(3004, function () {
    console.log("My quickfix api is running....");
});
module.exports = app;


