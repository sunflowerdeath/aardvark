import Index from './index.md'
import React from './react.md'

import Align from './elements/align.md'
import Sized from './elements/sized.md'
import Elements from './elements/elements.md'

import DesktopApplication from './desktop/desktop-application.md'
import DesktopWindow from './desktop/desktop-window.md'

import Scrollable from './components/scrollable.md'

export default {
	name: 'JS API',
	stories: {
	    'index': { name: 'Index', markdown: Index },
	    'react-renderer': { name: 'React renderer', markdown: React },
        'align-element': { name: 'AlignElement', markdown: Align },
        'sized-element': { name: 'SizedElement', markdown: Sized },
        'element': { name: 'Element', markdown: Elements },
        'desktop-application': {
            name: 'DesktopApplication', markdown: DesktopApplication
        },
        'desktop-window': { name: 'DesktopWindow', markdown: DesktopWindow },
        'scrollable': { name: 'Scrollable', markdown: Scrollable }
	}
}
