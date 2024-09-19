#!/bin/bash

# Variables
BLOCKCHAIN_URL="http://localhost:3000/query"
CHANNEL_ID="mychannel"
CHAINCODE_ID="basic"
QUERY_FUNCTION="GetAllAssets"

# Function to query the blockchain
query_blockchain() {
  echo "Querying blockchain for all assets"
  RESPONSE=$(curl -s -G --data-urlencode "channelid=$CHANNEL_ID" --data-urlencode "chaincodeid=$CHAINCODE_ID" --data-urlencode "function=$QUERY_FUNCTION" "$BLOCKCHAIN_URL")
  echo "Blockchain response: $RESPONSE"
}

# Main function to query and display values
main() {
  query_blockchain
}

main
