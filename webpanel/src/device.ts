type DeviceVersion = {
    firmware: string;
    idf: string;
}

export async function version(): Promise<DeviceVersion> {
    const url = '/device/version';
    const response = await fetch(url);

    if (!response.ok)
        throw Error(`GET ${url} failed: ${response.statusText}`);

    return await response.json();
}

export async function uptime(): Promise<number> {
    const url = '/device/uptime';
    const response = await fetch(url);

    if (!response.ok)
        throw Error(`GET ${url} failed: ${response.statusText}`);

    interface UptimeInterface {
        seconds: number;
    }

    return (await response.json() as UptimeInterface).seconds;
}

export async function reboot() {
    const url = '/device/reboot';
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