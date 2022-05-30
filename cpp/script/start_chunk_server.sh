chunk_server_exec="../cmake-build-debug-paperswagger/test/chunk_server_test/chunk_server"

for ((i=1;i<=3;++i)); do
  ${chunk_server_exec} ${i} \
  > "log_${i}" 2>&1 &
done
