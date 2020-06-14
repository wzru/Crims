const net = require('net');

const socket = new net.Socket();

const port = 8000;

//const hostname = '47.102.209.185';
const hostname = '127.0.0.1';

socket.setEncoding = 'UTF-8';

function sleep(sleepTime) {
  for (var start = new Date; new Date - start <= sleepTime;) { }
}

socket.connect(port, hostname, function () {
  // socket.write("INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100);");
  // sleep(500);
  socket.write("SELECT * FROM CAR_TYPE;");
  // sleep(500);
  // socket.write("SAVE;");
});

socket.on('data', function (msg) {
  console.log(msg.toString());
});

socket.on('error', function (error) {
  console.log('error' + error);
});

socket.on('close', function () {
  console.log('服务器端下线了');
});