# Crims

[![build](https://img.shields.io/badge/build-passing-brightgreen.svg?style=plastic-square)](https://github.com/WangZhengru/Crims) [![licence](https://badgen.net/github/license/WangZhengru/Crims)](https://github.com/WangZhengru/Crims) [![commits](https://badgen.net/github/commits/WangZhengru/Crims)](https://github.com/WangZhengru/Crims) [![last-commit](https://badgen.net/github/last-commit/WangZhengru/Crims)](https://github.com/WangZhengru/Crims) [![windows](https://badgen.net/badge/icon/windows?icon=windows&label)](https://github.com/WangZhengru/Crims)

[![cmake](https://badgen.net/badge/dependency/CMake/red)](https://github.com/WangZhengru/Crims) [![flex-bison](https://badgen.net/badge/dependency/Flex&Bison/orange)](https://github.com/WangZhengru/Crims) [![nodejs](https://badgen.net/badge/dependency/Node%2ejs/green)](https://github.com/WangZhengru/Crims)

"Car Rental Information Management System" from "Course Project of Programming" in HUST-CST-2020-Spring

只是一个课设玩具项目

## Features

项目采用前后端分离设计，后端使用纯C，前端使用React，并且用了一个node脚本来作为中间件（负责通信转发）

实现了一个简单的汽车租赁管理系统，特性如下：

+ 后端的数据存储在自定义格式的二进制文件中，启动时递归读取，保存时递归写入；读取数据之后存储在三个方向的十字链表上
+ 后端使用Flex & Bison构建了一个简易SQL解析器，并针对这个解析器实现了一个简易的SQL解释器，能够根据AST来执行SQL语句
+ 前后端使用socket进行通信，低耦合
+ 前端通过拼接SQL语句发送给后端执行来实现可视化的增删查改
+ 前端实现数据统计模块（输出数据统计表和数据统计图），并且实现xls文件的导入和导出

## Usage

需要依次启动server, router, client

```bash
cd Crims/
# 先启动server端, 默认监听8000端口. 必须要切换到server/bin目录下运行
cd server/bin
./crims_server.exe start &
# 再启动router端, 默认监听5000端口, 做socket转发
cd ../../router
node router.js &
# 最后启动client端(GUI), 默认监听3000端口
cd ../client
npm start
```

之后就可以在浏览器中的`http://localhost:3000`中访问到GUI

## Requirements

这个项目的完整编译运行依赖于flex, bison, cmake, node

 **服务端目前仅支持在Windows上运行**

## Developer

[WangZhengru](https://github.com/WangZhengru)

## License

[GPL-3.0](https://github.com/WangZhengru/Crims/blob/master/LICENSE) © Wang Zhengru