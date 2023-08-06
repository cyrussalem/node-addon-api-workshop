// eslint-disable-next-line @typescript-eslint/no-var-requires
const hello = require('bindings')('hello');

console.log(hello.hello());

const obj = {
    foo: "bar"
}


