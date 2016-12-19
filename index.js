var ansifilter = require('./build/Release/ansifilter.node')
function filter(s) {
	if (typeof s == 'undefined' || s === null)
		return s
	else
		return ansifilter.filter(s)
}
module.exports = filter

if (require.main == module) {
	const assert = require('assert')

	assert.equal(filter(), undefined)
	assert.equal(filter(null), null)
	assert.equal(filter('\u001b[?1034hHello World!'), 'Hello World!')
}
