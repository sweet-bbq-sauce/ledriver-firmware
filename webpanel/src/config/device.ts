export async function reboot() {
    const url = '/config/reboot';
    const response = await fetch(url, { method: 'POST' });

    if (!response.ok)
        throw Error(`GET ${url} failed: ${response.statusText}`);
}

export async function update() {
    const url = '/config/update';
    const response = await fetch(url, { method: 'POST' });

    if (!response.ok)
        throw Error(`GET ${url} failed: ${response.statusText}`);
}