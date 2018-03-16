importScripts('stdlibBundle.js');

const _console = console;

const stringify = value =>
  JSON.stringify(value) || String(value);

const send = (type, contents) =>
  postMessage({ type, contents });

const log = (type, items) =>
  send(type, items.map(stringify))

console = {
  log: (...items) => log('log', items),
  error: (...items) => log('error', items),
  warn: (...items) => log('warn', items),
};

onmessage = ({data}) => {
  eval(data.code);
  send('end', data.timerId);
};