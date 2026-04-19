import React from 'react';
import { createRoot } from 'react-dom/client';

const container = document.getElementById('root');
if (!container)
    throw new Error('Container not found');

interface PowerInterface {
    power: boolean;
}

interface ColorInterface {
    red: number;
    green: number;
    blue: number;
}

var color: ColorInterface = {
    red: 0,
    green: 0,
    blue: 0
}

var power = false;

var ws_rgb: WebSocket | null = null;

async function color_update(channel: 'red' | 'green' | 'blue', value: number) {
    color = { ...color, [channel]: value };

    if (ws_rgb) {
        const buffer = new ArrayBuffer(6);
        const view = new DataView(buffer);

        view.setUint16(0, color.red, false);
        view.setUint16(2, color.green, false);
        view.setUint16(4, color.blue, false);

        ws_rgb.send(buffer);
    }
}

async function power_update(value: boolean) {
    const json = JSON.stringify({ power: value });
    const response = await fetch('/control/power', {
        method: 'PUT',
        headers: { 'Content-Type': 'application/json' },
        body: json
    });

    if (response.ok)
        power = value;
    else alert('Power updating failed');
}

async function power_refresh() {
    const response = await fetch('/control/power');

    if (response.ok) {
        const json = (await response.json()) as PowerInterface;
        const input = document.getElementById('input_power') as HTMLInputElement;

        input.checked = json.power;
    }
    else alert('Power refreshing failed');
}

async function color_refresh() {
    const response = await fetch('/control/color');

    if (response.ok) {
        const json = (await response.json()) as ColorInterface;
        const input_red = document.getElementById('input_red') as HTMLInputElement;
        const input_green = document.getElementById('input_green') as HTMLInputElement;
        const input_blue = document.getElementById('input_blue') as HTMLInputElement;

        input_red.valueAsNumber = color.red = json.red;
        input_green.valueAsNumber = color.green = json.green;
        input_blue.valueAsNumber = color.blue = json.blue;
    }
    else alert('Color refreshing failed');
}

window.addEventListener('DOMContentLoaded', () => {
    const ws = new WebSocket('/control/rgb');
    ws.onopen = () => {
        power_refresh();
        color_refresh();

        ws_rgb = ws;
    }
});

createRoot(container).render(
    <React.StrictMode>
        <>
            <input
                id='input_red'
                type='range'
                min='0'
                max='65535'
                defaultValue={color.red}
                onChange={event => color_update('red', Number(event.target.value))}
            /><br/>
            <input
                id='input_green'
                type='range'
                min='0'
                max='65535'
                defaultValue={color.green}
                onChange={event => color_update('green', Number(event.target.value))}
            /><br/>
            <input
                id='input_blue'
                type='range'
                min='0'
                max='65535'
                defaultValue={color.blue}
                onChange={event => color_update('blue', Number(event.target.value))}
            /><br/>
            <input
                id='input_power'
                type='checkbox'
                defaultChecked={power}
                onChange={event => power_update(event.target.checked)}
            />
        </>
    </React.StrictMode>
);