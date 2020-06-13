const WebSocket = require("ws");
const WebSocketServer = WebSocket.Server;
const Net = require("net");

let globalWs;
let globalWs1;

// 作为服务端监听接收浏览器的的消息
const wss = new WebSocketServer({ port: 5000 });
wss.on("connection", function (ws) {
  console.log("wss连接上了浏览器");
  globalWs = ws;
  ws.on("message", function (msg) {
    console.log("ws接收到了来自浏览器的信息", msg);
    sc.write(msg);
  });
});

// 作为客户端监听C服务端的消息
const sc = new Net.Socket();
sc.setEncoding("utf-8");
sc.connect(8000, "127.0.0.1", function () {
  console.log("sc连接上了C服务器");
});
// 监听C来自服务器的data
sc.on("data", function (msg) {
  console.log("sc接收到了C服务器的数据", msg);
  // 给浏览器转发消息
  globalWs.send(msg);
});

// 作为服务端监听接收浏览器的的消息
const wss1 = new WebSocketServer({ port: 5001 });
wss1.on("connection", function (ws) {
  console.log("wss1连接上了浏览器");
  globalWs1 = ws;
  ws.on("message", function (msg) {
    console.log("ws1接收到了来自浏览器的信息", msg);
    sc1.write(msg);
  });
});

// 作为客户端监听C服务端的消息
const sc1 = new Net.Socket();
sc1.setEncoding("utf-8");
sc1.connect(8000, "127.0.0.1", function () {
  console.log("sc1连接上了C服务器");
});
// 监听C来自服务器的data
sc1.on("data", function (msg) {
  console.log("sc1接收到了C服务器的数据", msg);
  // 给浏览器转发消息
  globalWs1.send(msg);
});
