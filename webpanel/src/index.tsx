import React from 'react';
import { createRoot } from 'react-dom/client';

import * as color from './control/color';
import * as power from './control/power';
import * as utils from './utils';

interface ControlState {
    color: color.ColorState;
    power: boolean;
}

let state: ControlState = {
    color: await color.refresh(),
    power: await power.refresh()
};

const stream = new color.Stream();

const container = document.getElementById('root');
if (!container)
    throw new Error('Container not found');


async function update_channel(channel: 'red' | 'green' | 'blue', value: number) {
    state.color = { ...state.color, [channel]: value };
    stream.update(state.color);

    const picker = document.getElementById('input_picker');
    if (picker)
        (picker as HTMLInputElement).value = utils.u16_to_hex(state.color);
}

async function update_picker(hex: string) {
    state.color = utils.hex_to_u16(hex);
    stream.update(state.color);

    const input_red = document.getElementById('input_red');
    const input_green = document.getElementById('input_green');
    const input_blue = document.getElementById('input_blue');

    if (input_red)
        (input_red as HTMLInputElement).value = String(state.color.red);

    if (input_green)
        (input_green as HTMLInputElement).value = String(state.color.green);

    if (input_blue)
        (input_blue as HTMLInputElement).value = String(state.color.blue);
}

createRoot(container).render(
    <React.StrictMode>
        <>
            <input
                id='input_red'
                type='range'
                min='0'
                max='65535'
                defaultValue={state.color.red}
                onChange={event => update_channel('red', Number(event.target.value))}
            /><br />
            <input
                id='input_green'
                type='range'
                min='0'
                max='65535'
                defaultValue={state.color.green}
                onChange={event => update_channel('green', Number(event.target.value))}
            /><br />
            <input
                id='input_blue'
                type='range'
                min='0'
                max='65535'
                defaultValue={state.color.blue}
                onChange={event => update_channel('blue', Number(event.target.value))}
            /><br />
            <input
                id='input_power'
                type='checkbox'
                defaultChecked={state.power}
                onChange={event => power.update(event.target.checked)}
            /><br />
            <input
                type='color'
                id='input_picker'
                defaultValue={utils.u16_to_hex(state.color)}
                onChange={event => update_picker(event.target.value)}
            ></input>
        </>
    </React.StrictMode>
);