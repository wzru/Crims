const WebSocket = require("ws");
const WebSocketServer = WebSocket.Server;
const Net = require("net");

let globalWs, globalWs1, globalWs2, globalWs3, globalWs4;

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

// 作为服务端监听接收浏览器的的消息
const wss2 = new WebSocketServer({ port: 5002 });
wss2.on("connection", function (ws) {
  console.log("wss2连接上了浏览器");
  globalWs2 = ws;
  ws.on("message", function (msg) {
    console.log("ws2接收到了来自浏览器的信息", msg);
    sc2.write(msg);
  });
});

// 作为客户端监听C服务端的消息
const sc2 = new Net.Socket();
sc2.setEncoding("utf-8");
sc2.connect(8000, "127.0.0.1", function () {
  console.log("sc2连接上了C服务器");
});
// 监听C来自服务器的data
sc2.on("data", function (msg) {
  console.log("sc2接收到了C服务器的数据", msg);
  // 给浏览器转发消息
  globalWs2.send(msg);
});

// 作为服务端监听接收浏览器的的消息
const wss3 = new WebSocketServer({ port: 5003 });
wss3.on("connection", function (ws) {
  console.log("wss3连接上了浏览器");
  globalWs3 = ws;
  ws.on("message", function (msg) {
    console.log("ws3接收到了来自浏览器的信息", msg);
    sc3.write(msg);
  });
});

// 作为客户端监听C服务端的消息
const sc3 = new Net.Socket();
sc3.setEncoding("utf-8");
sc3.connect(8000, "127.0.0.1", function () {
  console.log("sc3连接上了C服务器");
});
// 监听C来自服务器的data
sc3.on("data", function (msg) {
  console.log("sc3接收到了C服务器的数据", msg);
  // 给浏览器转发消息
  globalWs3.send(msg);
});

// 作为服务端监听接收浏览器的的消息
const wss4 = new WebSocketServer({ port: 5004 });
wss4.on("connection", function (ws) {
  console.log("wss4连接上了浏览器");
  globalWs4 = ws;
  ws.on("message", function (msg) {
    console.log("ws4接收到了来自浏览器的信息", msg);
    sc4.write(msg);
  });
});

// 作为客户端监听C服务端的消息
const sc4 = new Net.Socket();
sc4.setEncoding("utf-8");
sc4.connect(8000, "127.0.0.1", function () {
  console.log("sc4连接上了C服务器");
});
// 监听C来自服务器的data
sc4.on("data", function (msg) {
  console.log("sc4接收到了C服务器的数据", msg);
  // 给浏览器转发消息
  globalWs4.send(msg);
});
