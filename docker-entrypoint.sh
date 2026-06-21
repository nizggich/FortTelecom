#!/bin/bash
set -e

echo "Statring server"
sudo /opt/ftelecom/server_app & 
SERVER_PID=$!

wait $SERVER_PID
