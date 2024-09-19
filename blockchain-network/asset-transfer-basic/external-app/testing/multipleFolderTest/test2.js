const { exec } = require('child_process');
const fs = require('fs');
const path = require('path');
const url = require('url');

// Array of Google Drive folder URLs with folder names as query parameters
const FOLDER_URLS = [
  'https://drive.google.com/drive/folders/15Zb_09CBI8EhQNgfxf_C8Z7axMsYCXot?folderName=Folder1',
  'https://drive.google.com/drive/folders/19fJK9_f3CxBOleXfa6h2gkT9L0hgTegT?folderName=Folder2',
  'https://drive.google.com/drive/folders/1J49xTvgD5UxdHfp1Ir2c9iQANfpnSc2d?folderName=Folder3',
  'https://drive.google.com/drive/folders/1Ojsy8Q5Il7kpsvs85M2famaq6mejeRNB?folderName=Folder4',
  'https://drive.google.com/drive/folders/1R8TA44GDjIzyJRMxFQMXEpwXtiJvDAc6?folderName=Folder5',
  'https://drive.google.com/drive/folders/1XmgSKpWHqi-Inmz_JDGgnlpA68BCJmUr?folderName=Folder6',
  'https://drive.google.com/drive/folders/1YRmzOyHnXAMf6eys9RH_fPmbcCexkexO?folderName=Folder7',
  'https://drive.google.com/drive/folders/1jwulBol6esgAk3fJsUMffmww-NpjL_fN?folderName=Folder8',
  'https://drive.google.com/drive/folders/1nBt26ZAnf68kzneiMlJjrNHClJ6I2ywi?folderName=Folder9',
  'https://drive.google.com/drive/folders/1tQzDVJwnT0CF15MW_Vwh-jbDXI75UWuU?folderName=Folder10'
];

// Function to download a Google Drive folder
function downloadFolder(url, dest) {
  return new Promise((resolve, reject) => {
    console.log(`Starting download for folder: ${url}`);
    const start = Date.now();
    exec(`gdown --folder ${url.split('?')[0]} -O ${dest}`, (error, stdout, stderr) => {
      const end = Date.now();
      const latency = end - start;
      console.log(`Download latency for ${path.basename(dest)}: ${latency} ms`);
      if (error) {
        reject(`Error: ${error.message}`);
        return;
      }
      if (stderr) {
        console.error(`stderr: ${stderr}`);
        // Not rejecting here as stderr might contain non-fatal errors/warnings
      }
      console.log(`Download completed for folder: ${url}`);
      resolve(latency);
    });
  });
}

// Function to parse the downloaded metadata file and extract values
function parseFile(filePath) {
  console.log(`Parsing metadata file: ${filePath}`);
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
    const latencies = [];
    for (const folderUrl of FOLDER_URLS) {
      const parsedUrl = new URL(folderUrl);
      const folderId = parsedUrl.pathname.split('/').pop(); // Extract folder ID from URL
      const folderName = parsedUrl.searchParams.get('folderName'); // Extract folder name from query parameter
      const outputDir = path.join(__dirname, 'downloaded_folder', folderName || folderId);

      // Create output directory if it doesn't exist
      if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
      }

      console.log(`Output directory for folder ${folderName || folderId} is ${outputDir}`);

      const latency = await downloadFolder(folderUrl, outputDir);
      latencies.push(latency);

      // Directly parse the metadata file
      const metadataFilePath = path.join(outputDir, 'metadata.json');
      if (fs.existsSync(metadataFilePath)) {
        parseFile(metadataFilePath);
      } else {
        console.error(`Metadata file not found in folder ${folderName || folderId}.`);
      }
    }

    // Calculate and log average latency
    const totalLatency = latencies.reduce((acc, curr) => acc + curr, 0);
    const averageLatency = totalLatency / latencies.length;
    console.log(`Average download latency: ${averageLatency.toFixed(2)} ms`);

  } catch (error) {
    console.error('Error in main execution:', error);
  }
})();
