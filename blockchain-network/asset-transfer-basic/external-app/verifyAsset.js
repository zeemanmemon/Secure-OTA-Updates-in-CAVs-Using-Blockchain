const axios = require('axios');
const fs = require('fs');
const path = require('path');


// URL of the Google Doc to download
const GOOGLE_DOC_URL = 'https://docs.google.com/document/d/1VEZwvZHxcIf_N-wsf_MjhEXd1nOv8prjisGsLerzxqI/export?format=txt';

// Function to download the Google Doc
async function downloadGoogleDoc(url, outputFilePath) {
  try {
    console.log(`Downloading Google Doc from ${url}...`);

    const response = await axios.get(url, { responseType: 'stream' });

    response.data.pipe(fs.createWriteStream(outputFilePath));

    response.data.on('end', () => {
      console.log(`Downloaded Google Doc to ${outputFilePath}`);
    });

  } catch (error) {
    console.error('Error downloading the Google Doc:', error);
  }
}

// Main execution
const outputFilePath = path.join(__dirname, 'GoogleDoc.txt');
downloadGoogleDoc(GOOGLE_DOC_URL, outputFilePath);



/*
/ Defined values
const ID = "1337";
const APPRAISED_VALUE = 5000;
const COLOR = "RED";
const OWNER = "ZEEMAN";
const SIZE = 4;

// ********** Querying the Blockchain **********
// Blockchain API details
const BLOCKCHAIN_URL = `http://localhost:3000/query?channelid=mychannel&chaincodeid=basic&function=ReadAsset&args=${ID}`;

// Function to query the blockchain and compare values
async function queryAndCompareBlockchain(id, appraisedValue, color, owner, size) {
  try {
    console.log(`Querying blockchain for asset ID ${id}...`);
    
    // Start timer
    console.time("Blockchain query time");

    const response = await axios.get(BLOCKCHAIN_URL);

    // End timer
    console.timeEnd("Blockchain query time");

    // Debug: Output the response
    console.log("Blockchain response:", response.data);

    // Removing the "Response: " part and parsing the JSON
    const jsonResponse = response.data.replace("Response: ", "");
    const blockchainAsset = JSON.parse(jsonResponse);

    const blockchainAppraisedValue = parseInt(blockchainAsset.AppraisedValue, 10);
    const blockchainColor = blockchainAsset.Color;
    const blockchainOwner = blockchainAsset.Owner;
    const blockchainSize = parseInt(blockchainAsset.Size, 10);

    // Debug: Output the parsed values
    console.log(`Expected values: AppraisedValue=${appraisedValue}, Color=${color}, Owner=${owner}, Size=${size}`);
    console.log(`Blockchain values: AppraisedValue=${blockchainAppraisedValue}, Color=${blockchainColor}, Owner=${blockchainOwner}, Size=${blockchainSize}`);

    // Compare values
    if (
      blockchainAppraisedValue === appraisedValue &&
      blockchainColor === color &&
      blockchainOwner === owner &&
      blockchainSize === size
    ) {
      console.log(`Asset ID ${id}: Not tampered`);
    } else {
      console.log(`Asset ID ${id}: Tampered`);
    }
  } catch (error) {
    console.error("Error querying the blockchain:", error);
  }
}

// Main execution
queryAndCompareBlockchain(ID, APPRAISED_VALUE, COLOR, OWNER, SIZE);
