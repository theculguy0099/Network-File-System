all:
	gcc naming_server.c naming_server_search_functions.c -o nm
	gcc -o client client.c retrieve.c write.c read.c makedir.c delete.c copy.c
	gcc storage_server.c -o ss 
