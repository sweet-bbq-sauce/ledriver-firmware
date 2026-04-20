import { ColorState } from './control/color';

export function hex_to_u16(hex: string): ColorState {
    const normalized = hex.replace('#', '');

    if (!/^[0-9a-fA-F]{6}$/.test(normalized))
        throw new Error('Expected hex in format #RRGGBB');

    const red8 = parseInt(normalized.slice(0, 2), 16);
    const green8 = parseInt(normalized.slice(2, 4), 16);
    const blue8 = parseInt(normalized.slice(4, 6), 16);

    return {
        red: Math.floor((red8 / 255) * 65535),
        green: Math.floor((green8 / 255) * 65535),
        blue: Math.floor((blue8 / 255) * 65535)
    };
}

export function u16_to_hex(rgb: ColorState): string {
    const values = [rgb.red, rgb.green, rgb.blue];

    for (const v of values) {
        if (!Number.isInteger(v) || v < 0 || v > 65535) {
            throw new Error('Expected uint16 values in range 0-65535');
        }
    }

    return (
        '#' +
        values
            .map((v) => Math.round((v / 65535) * 255))
            .map((v) => v.toString(16).padStart(2, '0'))
            .join('')
            .toUpperCase()
    );
}