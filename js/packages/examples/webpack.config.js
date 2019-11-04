const path = require('path')

module.exports = {
	entry: './src/index.js',
	output: {
		path: path.resolve(__dirname, 'build'),
		filename: 'main.js'
	},
	mode: process.env.NODE_ENV == 'production' ? 'production' : 'development',
	module: {
		rules: [
			{
				test: /\.js$/,
				exclude: /node_modules/,
				use: {
					loader: 'babel-loader',
					options: {
						presets: ['@babel/preset-react']
					}
				}
			}
		]
	},
    devtool: 'source-map'
}
