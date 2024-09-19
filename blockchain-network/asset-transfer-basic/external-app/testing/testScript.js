const { exec } = require('child_process');
const fs = require('fs');
const path = require('path');

// Folder URL of the Google Drive folder
const FOLDER_URL = 'https://drive.google.com/drive/folders/1AscuEszzSk_jP2C8BME62wJsMdrOhuNJ';

// Function to download the Google Drive folder
function downloadFolder(url, dest) {
  return new Promise((resolve, reject) => {
    console.time('Download latency');
    exec(`gdown --folder ${url} -O ${dest}`, (error, stdout, stderr) => {
      console.timeEnd('Download latency');
      if (error) {
        reject(`Error: ${error.message}`);
        return;
      }
      if (stderr) {
        console.error(`stderr: ${stderr}`);
        // Not rejecting here as stderr might contain non-fatal errors/warnings
      }
      resolve(stdout);
    });
  });
}

// Function to parse the downloaded metadata file and extract values
function parseFile(filePath) {
  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error('Error reading the file:', err);
      return;
    }

    try {
      const metadata = JSON.parse(data);

      // Variables to store the parsed values
      const { ID, AppraisedValue, Color, Owner, Size } = metadata;

      // Log the parsed values
      console.log('Parsed values:');
      console.log('ID:', ID);
      console.log('AppraisedValue:', AppraisedValue);
      console.log('Color:', Color);
      console.log('Owner:', Owner);
      console.log('Size:', Size);

      // Use the variables as needed
    } catch (parseError) {
      console.error('Error parsing JSON:', parseError);
    }
  });
}

// Main execution
(async () => {
  try {
    const outputDir = path.join(__dirname, 'downloaded_folder');
    const result = await downloadFolder(FOLDER_URL, outputDir);
    console.log(result);

    // Directly parse the metadata file
    const metadataFilePath = path.join(outputDir, 'metadata.json');
    if (fs.existsSync(metadataFilePath)) {
      parseFile(metadataFilePath);
    } else {
      console.error('Metadata file not found.');
    }
  } catch (error) {
    console.error('Error in main execution:', error);
  }
})();
