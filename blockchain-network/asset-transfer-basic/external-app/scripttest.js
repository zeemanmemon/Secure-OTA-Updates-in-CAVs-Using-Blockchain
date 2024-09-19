const fs = require('fs');
const fetch = require('node-fetch');
const { execSync } = require('child_process');
// https://drive.google.com/file/d/1J8aecfX8DjamTodc1uptVCHaJ2MIuKgN/view?usp=sharing
// Google Drive direct download URL
const FILE_ID = '1J8aecfX8DjamTodc1uptVCHaJ2MIuKgN'; // Replace with your Google Drive file ID
const OTA_UPDATE_URL = `https://drive.google.com/uc?export=download&id=${FILE_ID}`;
const DESTINATION = './downloaded_file.txt';

// Function to download file using curl
async function downloadFile(url, destination) {
    try {
        execSync(`curl -L -o ${destination} ${url}`);
        console.log('File downloaded successfully.');
    } catch (error) {
        console.error('Error downloading the file:', error);
    }
}

// Function to extract ID and AppraisedValue from the text file
function extractValues(filePath) {
    try {
        const data = fs.readFileSync(filePath, 'utf8');
        const lines = data.split('\n');
        let id, appraisedValue;
        for (const line of lines) {
            if (line.startsWith('ID:')) {
                id = line.split(':')[1].trim();
            } else if (line.startsWith('AppraisedValue:')) {
                appraisedValue = parseInt(line.split(':')[1].trim(), 10);
            }
        }
        if (!id || !appraisedValue) {
            throw new Error('ID or AppraisedValue not found in the file.');
        }
        return { id, appraisedValue };
    } catch (err) {
        console.error('Error reading the file:', err);
    }
}

// Function to query the blockchain
async function queryBlockchain(assetID) {
    try {
        const response = await fetch(`http://localhost:3000/query?channelid=mychannel&chaincodeid=basic&function=ReadAsset&args=${assetID}`, {
            method: 'GET',
        });
        const result = await response.json();
        return result;
    } catch (error) {
        console.error('Error querying the blockchain:', error);
    }
}

// Function to compare values and print the result
async function compareValues(filePath) {
    const { id, appraisedValue } = extractValues(filePath);
    const blockchainAsset = await queryBlockchain(id);

    if (blockchainAsset.AppraisedValue === appraisedValue) {
        console.log('Not tampered');
    } else {
        console.log('Tampered');
    }
}

// Main function to download, extract, and compare values
async function main() {
    await downloadFile(OTA_UPDATE_URL, DESTINATION);
    await compareValues(DESTINATION);
}

main().catch(console.error);
