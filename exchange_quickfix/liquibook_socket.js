const server = require('http').createServer();
const io = require('socket.io')(server, {origins: '*:*'});
//io.origins('*:*')

var mysql = require('mysql');

var con = mysql.createConnection({
    host: "18.223.20.192",
    user: "root",
    password: "32612^#HJD*^Gi27))863eydh8o",
    database: "chainbyte_exchange"
});

con.connect(function (err) {
    if (err)
        throw err;
    console.log("Connected!");

});

io.on('connection', function (socket) {
    console.log("connection done.");
    //socket.emit('news', { hello: 'world' });
    socket.on('order_status', function (data) {

        var order_data = JSON.parse(data);

 console.log(order_data);

        //check order status
        if (order_data['39'] != 0) {

            switch (order_data['39']) {
                //in process orders
                case 1:
                    var sql = "UPDATE zozocoinex_orders SET status='Processing', market_amount=" +order_data['38'] + "  WHERE unique_order_id=" + order_data['11'];
                    con.query(sql, function (err, result) {
                        if (err)
                            throw err;
			 console.log(sql);
                        console.log("1 record updated");

                    });
                    break;
                case 2:
                    var sql = "UPDATE zozocoinex_orders SET status='Completed' WHERE unique_order_id=" + order_data['11'];
                    con.query(sql, function (err, result) {
                        if (err)
                            throw err;
                        console.log("1 record updated");
                    });
                    break;
                case 4:
                    var sql = "UPDATE zozocoinex_orders SET status='Cancelled' WHERE unique_order_id=" + order_data['11'];
                    con.query(sql, function (err, result) {
                        if (err)
                            throw err;
                        console.log("1 record updated");
                    });
                    break;
                default:

                    break;
            }


            /*var sql = "INSERT INTO order_status_socket_data (order_data) VALUES ('" + order_data + "')";
             con.query(sql, function (err, result) {
             if (err)
             throw err;
             console.log("1 record inserted");
             });*/

        }

    });
});
server.listen(3005);