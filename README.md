# Crims

[![build](https://img.shields.io/badge/build-passing-brightgreen.svg?style=plastic-square)](https://github.com/wzru/Crims) [![licence](https://badgen.net/github/license/wzru/Crims)](https://github.com/wzru/Crims) [![commits](https://badgen.net/github/commits/wzru/Crims)](https://github.com/wzru/Crims) [![last-commit](https://badgen.net/github/last-commit/wzru/Crims)](https://github.com/wzru/Crims)

[![cmake](https://badgen.net/badge/dependency/CMake/red)](https://cmake.org/) [![flex-bison](https://badgen.net/badge/dependency/Flex/orange)](https://www.gnu.org/software/flex/) [![flex-bison](https://badgen.net/badge/dependency/Bison/pink)](https://www.gnu.org/software/bison/) [![nodejs](https://badgen.net/badge/dependency/Node%2ejs/green)](https://nodejs.org/)

"Car Rental Information Management System" from "Course Project of Programming" in HUST-CSE-2020

C语言&数据结构&编译原理课设的toy project

## Features

项目采用前后端分离设计，后端使用纯C，前端使用React，并且用了一个node脚本来作为中间件（负责通信转发）

实现了一个简单的汽车租赁信息管理系统，特性如下：

+ 后端的数据存储在自定义格式的二进制文件中，启动时递归读取，保存时递归写入；读取数据之后在内存中存储在三个方向的十字链表上
+ 后端使用**Flex & Bison**构建了一个简易SQL解析器，并针对这个解析器实现了一个简易的SQL解释器，能够根据AST来执行SQL语句
+ 前后端使用socket进行通信，低耦合；由于JS不支持原生socket，所以使用了一个node脚本来作为中间件通信转发。
+ 前端通过拼接SQL语句发送给后端执行来实现可视化的增删查改
+ 前端实现数据统计模块（输出数据统计表和数据统计图），并且支持xls文件的导入和导出

## Usage

需要依次启动server, router, client

依次打开三个终端：

1. 启动server服务端

```bash
# 先启动server端, 默认监听8000端口. 必须要切换到server/bin目录下运行
cd Crims/server/bin
./crims_server.exe start
```

2. 启动router中间件

```bash
# 再启动router端, 做socket转发
cd Crims/router
node router.js
```

3. 启动client客户端

```bash
# 最后启动client端(GUI), 默认监听3000端口
cd Crims/client
npm install && npm start
```

之后就可以在浏览器中的`http://localhost:3000`中访问到客户端

## Requirements

这个项目的完整编译/运行依赖于Flex, Bison, CMake, Node.js

## Contributors

[wzru](https://github.com/wzru)

## License

[GPL-3.0](https://github.com/wzru/Crims/blob/master/LICENSE) © [wzru](https://github.com/wzru)