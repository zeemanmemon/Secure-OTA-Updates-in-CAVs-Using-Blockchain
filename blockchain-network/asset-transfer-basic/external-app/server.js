
// Sets up an Express server to serve the static files and handle API requests, e.g., serve Static Files: Serves HTML, CSS, and JS files from the public directory, and parse incoming request bodies in a middleware before handlers.

const express = require('express');
const path = require('path');

const app = express();
const PORT = 5000;

// Serve static files from the "public" directory
app.use(express.static(path.join(__dirname, 'public')));

app.listen(PORT, () => {
    console.log(`Server is listening on http://localhost:${PORT}`);
});
