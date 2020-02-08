let path = require('path')
let idl = require('../../jsi/idl')

let src = ['base_types', 'desktop_app', 'desktop_window', 'document'].map(
    file => path.resolve(__dirname, `../api/${file}.yaml`))

idl({
    src,
    defaultNamespace: 'aardvark',
    output: {
        dir: path.resolve(__dirname, '../generated'),
        filename: 'api',
    // TODO until core moved to nested namespace `aardvark::ui` or 
    // `aardvark::core` to prevent name conflicts between aardvark::Value 
    // and aardvark::jsi::Value
        namespace: 'aardvark_js_api',
        classname: `Api`
    }
})
