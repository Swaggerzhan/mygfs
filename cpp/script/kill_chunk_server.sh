proc_name="chunk_server"

pid=`ps -ef | grep ${proc_name} | grep -v grep | awk '{print $2}'`
pids=(${pid// / })

for ((i=0;i<3;++i)) do
  kill ${pids[${i}]}
  echo "kill ${pids[${i}]}"
done