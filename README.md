# UNIX File Server Instructions

REAL INSTRUCTIONS
1. Open CSX in two windows. Designate a "server" and a "client".
2. Run serverScriptv1, and clientScriptv1
3. Designate which file from server should be sent to the local file
4. Hit enter on server to kill server!






SIMULATION INSTRUCTIONS
1. In one window, navigate to file server and compile the server.c and client.c using the following commands:
   
  gcc server.c -o server
  gcc client.c -o client
  
2. Next, make the server file run in the background using:

   ./server &

3. Run the client file and use the two argument:

  ./client 127.0.0.1 testfile.txt downloaded.txt  (testfile contents -> overwrites downloaded)

4. Finally, make sure to kill the server when youre done!

  pkill server
