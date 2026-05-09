import React, { useState, useEffect } from 'react';
import { createRoot } from 'react-dom/client';

import * as device from './device';
import * as color from './control/color';
import * as power from './control/power';
import * as utils from './utils';

interface ControlState {
    color: color.ColorState;
    power: boolean;
}

const App: React.FC = () => {
    const [state, setState] = useState<ControlState>({
        color: { red: 0, green: 0, blue: 0 },
        power: false
    });
    const [stream] = useState(() => new color.Stream());

    useEffect(() => {
        const init = async () => {
            try {
                const colorState = await color.refresh();
                const powerState = await power.refresh();
                setState({ color: colorState, power: powerState });
                console.log(await device.version());
                console.log(`Uptime: ${await device.uptime()}s`);
            } catch (err) {
                console.error('Initialization error:', err);
            }
        };
        init();
    }, []);

    const updateChannel = async (channel: 'red' | 'green' | 'blue', value: number) => {
        const newColor = { ...state.color, [channel]: value };
        setState(prev => ({ ...prev, color: newColor }));
        stream.update(newColor);
    };

    const updatePicker = (hex: string) => {
        const newColor = utils.hex_to_u16(hex);
        setState(prev => ({ ...prev, color: newColor }));
        stream.update(newColor);
    };

    const updatePower = async (checked: boolean) => {
        setState(prev => ({ ...prev, power: checked }));
        await power.update(checked);
    };

    const handleUpdate = async () => {
        try {
            await device.update();
            alert('Device is going to check update. This can cause reboot.');
        } catch (err) {
            console.error(`Firmware update error: ${err}`);
            alert('Firmware updating failed.');
        }
    };

    const handleReboot = async () => {
        try {
            await device.reboot();
            alert('Device is going to reboot in 5s.');
            setTimeout(() => location.reload(), 7000);
        } catch (err) {
            console.error(`Device reboot error: ${err}`);
            alert('Device can\'t reboot.');
        }
    };

    return (
        <>
            <div>
                <label>Red:</label>
                <input
                    type='range'
                    min='0'
                    max='65535'
                    value={state.color.red}
                    onChange={e => updateChannel('red', Number(e.target.value))}
                />
                <span>{state.color.red}</span>
            </div>
            <div>
                <label>Green:</label>
                <input
                    type='range'
                    min='0'
                    max='65535'
                    value={state.color.green}
                    onChange={e => updateChannel('green', Number(e.target.value))}
                />
                <span>{state.color.green}</span>
            </div>
            <div>
                <label>Blue:</label>
                <input
                    type='range'
                    min='0'
                    max='65535'
                    value={state.color.blue}
                    onChange={e => updateChannel('blue', Number(e.target.value))}
                />
                <span>{state.color.blue}</span>
            </div>
            <div>
                <label>Power:</label>
                <input
                    type='checkbox'
                    checked={state.power}
                    onChange={e => updatePower(e.target.checked)}
                />
            </div>
            <div>
                <label>Color Picker:</label>
                <input
                    type='color'
                    value={utils.u16_to_hex(state.color)}
                    onChange={e => updatePicker(e.target.value)}
                />
            </div>
            <button onClick={handleUpdate}>Check for updates</button>
            <button onClick={handleReboot}>Reboot device</button>
        </>
    );
};

const container = document.getElementById('root');
if (!container)
    throw new Error('Container not found');

createRoot(container).render(
    <React.StrictMode>
        <App />
    </React.StrictMode>
);