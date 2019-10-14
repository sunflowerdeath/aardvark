import { useEffect, useRef } from 'react'

// When you pass somewhere value of state or props, it will not be
// updated automatically. That hooks wrap value into getter, that always
// returns last passed value.
const useLastValue = value => {
	const ref = useRef(value)
    if (ref.current !== value) ref.current = value
	return () => ref.current
}

export default useLastValue
