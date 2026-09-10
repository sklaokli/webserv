#!/bin/bash
printf "Content-Type: text/plain\r\n\r\n"
echo "Hello from Bash CGI!"
echo "REQUEST_METHOD: $REQUEST_METHOD"
echo "QUERY_STRING: $QUERY_STRING"
echo "SERVER_NAME: $SERVER_NAME"
echo "SERVER_PORT: $SERVER_PORT"

