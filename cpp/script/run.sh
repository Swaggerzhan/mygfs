chunk_server_exec="../cmake-build-debug-paperswagger/test/chunk_server_test/chunk_server"

for ((i=30001;i<=30003;++i)); do
  ${chunk_server_exec} ${i} \
  > "log_${i}" 2>&1 &
done


master_server_exec="../cmake-build-debug-paperswagger/test/master_server_test/master_server"

${master_server_exec}