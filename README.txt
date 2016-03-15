Code created by Artur Pimentel de Oliveira, Andrew Steinberg and 
Tabitha Rickets for assignment 3 in Computer Networks - University of
Notre Dame

After compiled to a executable called TCPserver (be sure to compile
using the library mhash), use the following command to run the program:

	$ ./TCPserver [port number]

Server can be quitted normally in the command prompt by using the Ctrl+C key

------------------------------------------------------------------------
After compiled to a executable called TCPclient (be sure to compile
using the library mhash), use the following command to run the program:

	$ ./TCPclient [server host] [port number] [file name]

This client will copy the file [file name] from the TCPserver dirctory
to the TCPclient directory. MD5 hashs are checked to ensure file's 
integrity