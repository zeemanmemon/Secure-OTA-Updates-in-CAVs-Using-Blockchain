


/**
Purpose:
Handles form submissions and interacts with the server to create and query assets on the blockchain.

Key Functions:
Create Asset: Sends a POST request to the server with asset details.
Query Asset: Sends a GET request to the server to query asset details.

 */
document.getElementById('createAssetForm').addEventListener('submit', async (event) => {
    event.preventDefault();
    const assetID = document.getElementById('assetID').value;
    const color = document.getElementById('color').value;
    const size = document.getElementById('size').value;
    const owner = document.getElementById('owner').value;
    const appraisedValue = document.getElementById('appraisedValue').value;

    const data = new URLSearchParams();
    data.append('channelid', 'mychannel');
    data.append('chaincodeid', 'basic');
    data.append('function', 'createAsset');
    data.append('args', assetID);
    data.append('args', color);
    data.append('args', size);
    data.append('args', owner);
    data.append('args', appraisedValue);

    try {
        const response = await fetch('http://localhost:3000/invoke', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: data,
        });
        const result = await response.text();
        document.getElementById('response').innerText = `Response: ${result}`;
    } catch (error) {
        console.error('Error:', error);
        document.getElementById('response').innerText = `Error: ${error}`;
    }
});

document.getElementById('queryAssetForm').addEventListener('submit', async (event) => {
    event.preventDefault();
    const assetID = document.getElementById('queryAssetID').value;

    try {
        const response = await fetch(`http://localhost:3000/query?channelid=mychannel&chaincodeid=basic&function=ReadAsset&args=${assetID}`, {
            method: 'GET',
        });
        const result = await response.json();
        document.getElementById('response').innerText = `Response: ${JSON.stringify(result, null, 2)}`;
    } catch (error) {
        console.error('Error:', error);
        document.getElementById('response').innerText = `Error: ${error}`;
    }
});
