import React, { useState, useCallback } from 'react'
import { Value, Color, TransformMatrix } from '@advk/common'
import ReactAardvark, {
	Align,
	Sized,
	Stack,
	Background,
	Layer
} from '@advk/react-renderer'

const win = application.createWindow(640, 480)
const document = application.getDocument(win)

const toRad = deg => deg * Math.PI / 180

// const matrix = TransformMatrix.identity
const matrix = TransformMatrix.makeRotate(toRad(45), 50, 50)

const App = () => (
	<Stack>
		<Background color={Color.WHITE} />
		<Align align={{ top: Value.abs(50), left: Value.abs(50) }}>
			<Layer transform={matrix}>
				<Sized
					sizeConstraints={{
						width: Value.abs(100),
						height: Value.abs(100)
					}}
				>
					<Background color={Color.RED} />
				</Sized>
			</Layer>
		</Align>
	</Stack>
)

ReactAardvark.render(<App />, document)
