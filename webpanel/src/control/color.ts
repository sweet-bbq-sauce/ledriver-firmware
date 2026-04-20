export type ColorState = {
    red: number;
    green: number;
    blue: number;
}

export class Stream {
    private ws_rgb: WebSocket | null = null;

    constructor() {
        const uri = `${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/control/rgb`;
        const ws = new WebSocket(uri);
        ws.onopen = () => {
            this.ws_rgb = ws;
        }
    }

    public update(value: ColorState) {
        if (this.ws_rgb && this.ws_rgb.readyState == this.ws_rgb.OPEN) {
            const buffer = new ArrayBuffer(6);
            const view = new DataView(buffer);

            view.setUint16(0, value.red, false);
            view.setUint16(2, value.green, false);
            view.setUint16(4, value.blue, false);

            this.ws_rgb.send(buffer);
        }
    }
}

export async function refresh(): Promise<ColorState> {
    const url = '/control/color';
    const response = await fetch(url);

    if (response.ok) {
        const color: ColorState = await response.json();
        return color;
    }

    throw Error(`GET ${url} failed: ${response.statusText}`);
}

export async function update(color: ColorState): Promise<void> {
    const url = '/control/color';
    const response = await fetch(url, {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(color)
    });

    if (response.ok)
        return;

    throw Error(`PUT ${url} failed: ${response.statusText}`);
}
