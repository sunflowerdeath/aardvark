const path = require('path')

module.exports = {
	entry: './src/react.js',
	output: {
		path: path.resolve(__dirname, 'build'),
		filename: 'main.js'
	},
	mode: 'development',
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
	}
}
