gcc server.c -o write_server
gcc server.c -D READ_SERVER -o read_server

I use an "check" array to memorize the file descriptor which need to check if it is ready.
lock function is used to lock the entire file.
