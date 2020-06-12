#!bash
path=$(dirname $(readlink -f "$0"))
cd ${path}/server/bin
./crims_server start -p 8000 &
node ../../router/router.js &
# trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT