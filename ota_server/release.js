const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { execFileSync } = require('child_process');

const VERSION = process.argv[2];
if (typeof VERSION != 'string') {
    console.error('release.js [version]');
    process.exit(1);
}

const firmware_src_path = path.join(__dirname, '../build/ledriver-firmware.bin');
const firmware_public_path = path.join(__dirname, 'public/firmware.bin');
const webpanel_src_path = path.join(__dirname, '../build/webpanel.bin');
const webpanel_public_path = path.join(__dirname, 'public/webpanel.bin');
const manifest_path = path.join(__dirname, 'public/manifest.json');

fs.cpSync(firmware_src_path, firmware_public_path);
fs.cpSync(webpanel_src_path, webpanel_public_path);

const esptool_output = execFileSync('python3', ['-m', 'esptool', 'image_info', firmware_public_path], { encoding: 'utf8' });

const firmware_sha256 = esptool_output.match(/Validation Hash:\s*([a-f0-9]{64})/i)[1].toLowerCase();
const webpanel_sha256 = crypto.createHash('sha256').update(fs.readFileSync(webpanel_public_path)).digest('hex');

const manifest = {
    firmware: {
        version: VERSION,
        sha256: firmware_sha256,
        path: '/firmware.bin'
    },
    webpanel: {
        version: VERSION,
        sha256: webpanel_sha256,
        path: '/webpanel.bin'
    }
};

fs.writeFileSync(manifest_path, JSON.stringify(manifest), 'utf8');
console.log('New manifest: ', manifest);