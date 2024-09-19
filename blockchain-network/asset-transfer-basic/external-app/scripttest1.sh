#!/bin/bash

# Variables
GOOGLE_DRIVE_FILE_ID="1J8aecfX8DjamTodc1uptVCHaJ2MIuKgN"
DESTINATION_FILE="./downloaded_file.txt"
BLOCKCHAIN_URL="http://localhost:3000/query"
CHANNEL_ID="mychannel"
CHAINCODE_ID="basic"
QUERY_FUNCTION="ReadAsset"

# Function to download file from Google Drive
download_file() {
  echo "Downloading file from Google Drive..."
  curl -L -o $DESTINATION_FILE "https://drive.google.com/uc?export=download&id=$GOOGLE_DRIVE_FILE_ID"
  echo "File downloaded successfully to $DESTINATION_FILE."
}

# Function to extract values from the text file
extract_values() {
  echo "Extracting values from the file..."
  local id
  local appraised_value
  local color
  local owner
  local size

  while IFS= read -r line; do
    echo "Processing line: $line"  # Debugging line
    if [[ $line == ID:* ]]; then
      id=$(echo $line | cut -d':' -f2 | xargs)
      echo "ID extracted: $id"  # Debugging line
    elif [[ $line == AppraisedValue:* ]]; then
      appraised_value=$(echo $line | cut -d':' -f2 | xargs)
      echo "AppraisedValue extracted: $appraised_value"  # Debugging line
    elif [[ $line == Color:* ]]; then
      color=$(echo $line | cut -d':' -f2 | xargs)
      echo "Color extracted: $color"  # Debugging line
    elif [[ $line == Owner:* ]]; then
      owner=$(echo $line | cut -d':' -f2 | xargs)
      echo "Owner extracted: $owner"  # Debugging line
    elif [[ $line == Size:* ]]; then
      size=$(echo $line | cut -d':' -f2 | xargs)
      echo "Size extracted: $size"  # Debugging line
      # Once all values are extracted, query the blockchain
      query_and_compare_blockchain $id $appraised_value $color $owner $size
    fi
  done < $DESTINATION_FILE
}

# Function to query the blockchain and compare values
query_and_compare_blockchain() {
  local id=$1
  local appraised_value=$2
  local color=$3
  local owner=$4
  local size=$5

  echo "Querying blockchain for asset ID $id..."
  RESPONSE=$(curl -s -G --data-urlencode "channelid=$CHANNEL_ID" --data-urlencode "chaincodeid=$CHAINCODE_ID" --data-urlencode "function=$QUERY_FUNCTION" --data-urlencode "args=$id" "$BLOCKCHAIN_URL")
  
  # Debug: Output the response
  echo "Blockchain response: $RESPONSE"
  
  # Parsing the response
  BLOCKCHAIN_APPRAISED_VALUE=$(echo $RESPONSE | jq -r '.[0].AppraisedValue')
  BLOCKCHAIN_COLOR=$(echo $RESPONSE | jq -r '.[0].Color')
  BLOCKCHAIN_OWNER=$(echo $RESPONSE | jq -r '.[0].Owner')
  BLOCKCHAIN_SIZE=$(echo $RESPONSE | jq -r '.[0].Size')

  # Check if parsing was successful
  if [[ "$BLOCKCHAIN_APPRAISED_VALUE" == "null" || "$BLOCKCHAIN_COLOR" == "null" || "$BLOCKCHAIN_OWNER" == "null" || "$BLOCKCHAIN_SIZE" == "null" ]]; then
    echo "Error: Unable to parse blockchain response."
    exit 1
  fi

  # Compare values
  if [[ "$BLOCKCHAIN_APPRAISED_VALUE" == "$appraised_value" && "$BLOCKCHAIN_COLOR" == "$color" && "$BLOCKCHAIN_OWNER" == "$owner" && "$BLOCKCHAIN_SIZE" == "$size" ]]; then
    echo "Asset ID $id: Not tampered"
  else
    echo "Asset ID $id: Tampered"
  fi
}

# Main execution
download_file
extract_values
