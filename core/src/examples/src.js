let app = new DesktopApp()
log('create app')
let window = app.createWindow(640, 480)
log('create window')
log('app.windows.size =', app.windows.size)
log('app.windows[0].width =', app.windows[0].width)
let doc = app.getDocument(window)
log('get doc')

let stack = new Stack()

let align = new Align()
align.align = {
    left: {type: 'abs', value: 50},
    top: {type: 'abs', value: 100}
}

let size = new Sized()
size.sizeConstraints = {
    minWidth: {type: 'abs', value: 200},
    maxWidth: {type: 'abs', value: 200},
    minHeight: {type: 'abs', value: 50},
    maxHeight: {type: 'abs', value: 50}
}

let red = new Background()

size.appendChild(red)
align.appendChild(size)
stack.appendChild(align)

let text = new Text("Hello World")
log(text.text)
text.text = "Hello, Text"
stack.appendChild(text)

/*
let isPressed = false;
let responder = new GestureResponder();
responder.onStart = event => {
    if (event.type === 'pointer_down') isPressed = true
}
responder.onUpdate = event => {
}
responder.onEnd = (event, isTerminated) => {
    if (isPressed && event.type === 'pointer_up') handleClick();
    pressed = false
}
*/

log(JSON.stringify(align.align))

doc.root = stack
log('set root')

let removeHandler
let handler = event => {
    log('pointer event ' + JSON.stringify(event))
    if (event.action == 3) removeHandler()
}
removeHandler = doc.addHandler(handler)

let stopTracking
let trackHandler = event => {
    log('tracked pointer ' + JSON.stringify(event))
    if (event.action == 3) stopTracking()
}
stopTracking = doc.startTrackingPointer(0, trackHandler)

app.run()
