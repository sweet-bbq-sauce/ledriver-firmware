const path = require('path');
const fs = require('fs');

const express = require('express');
const morgan = require('morgan');

const HOSTNAME = process.argv[2];
const PORT = process.argv[3];

if (typeof HOSTNAME != 'string' || typeof PORT != 'string') {
    console.error('server.js [hostname] [port]');
    process.exit(1);
}

const app = express();
app.use(morgan('dev'));
app.use(express.static(path.join(__dirname, 'public')));
app.listen(Number(PORT), HOSTNAME);