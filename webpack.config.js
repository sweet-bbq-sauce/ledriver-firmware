const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CompressionPlugin = require('compression-webpack-plugin');

module.exports = (_, argv = {}) => {
    const isProd = argv.mode === 'production';

    return {
        mode: isProd ? 'production' : 'development',
        entry: './webpanel/index.tsx',
        output: {
            path: path.resolve(__dirname, 'webpanel/dist'),
            filename: 'bundle.js',
            clean: true
        },
        resolve: {
            extensions: ['.ts', '.tsx', '.js']
        },
        module: {
            rules: [
                {
                    test: /\.tsx?$/,
                    use: 'ts-loader',
                    exclude: /node_modules/
                }
            ]
        },
        plugins: [
            new HtmlWebpackPlugin({
                template: './webpanel/public/index.html'
            }),
            ...(isProd ? [
                new CompressionPlugin({
                    algorithm: 'gzip',
                    test: /\.(js|css|html|svg)$/,
                    compressionOptions: { level: 9 },
                    filename: '[path][base].gz',
                    deleteOriginalAssets: true
                })
            ] : [])
        ],
        devServer: {
            static: './webpanel/dist',
            port: 3000,
            open: true,
            hot: true,
            compress: true
        }
    };
};
