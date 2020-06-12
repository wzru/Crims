# Crims Server

## Usage

This project uses cmake and make. Go check them out if you don't have them locally installed. 

```shell
# 必须切换到bin目录下运行
cd Crims/server/bin
./crims_server start -p 8000 # start server on port 8000
./crims_server --help # get help
./crims_server --version # get version
./crims_server shell# interactive shell
./crims_server exec "some commands;" # execute some commands
```

## Schedule

- [x] Basic Data Structures
- [x] File Storage
- [x] Windows Socket
- [x] Multithreading
- [x] SQL Parser
- [x] SQL Executor
- [x] JSON
- [x] Help Document
- [x] Test
