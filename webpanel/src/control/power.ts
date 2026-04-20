type PowerState = {
    on: boolean;
}

export async function refresh(): Promise<boolean> {
    const url = '/control/power';
    const response = await fetch(url);

    if (response.ok) {
        const power: PowerState = await response.json();
        return power.on;
    }

    throw Error(`GET ${url} failed: ${response.statusText}`);
}

export async function update(on: boolean): Promise<void> {
    const url = '/control/power';
    const response = await fetch(url, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ on: on })
    });

    if (response.ok)
        return;

    throw Error(`PUT ${url} failed: ${response.statusText}`);
}
