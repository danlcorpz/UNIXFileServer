# UNIX File Server Instructions

1. In one window, navigate to file server and compile the server.c and client.c using the following commands:
   
  gcc server.c -o server
  gcc client.c -o client
  
2. Next, make the server file run in the background using:

   ./server &

3. Finally, run the client file and use the two argument:

  ./client 127.0.0.1 testfile.txt downloaded.txt 
  
