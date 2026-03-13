const path = require('path');
const fs = require('fs');

const express = require('express');
const morgan = require('morgan');

const app = express();
app.use(morgan('dev'));
app.use(express.static(path.join(__dirname, "../dist")));

app.get('/manifest.json', (req, res) => {
    res.json({
        test: 420
    });
})

app.listen(8888, '0.0.0.0');