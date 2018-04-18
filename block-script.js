(function() {

  var node = (tag, attrs, children) => {
    var node = document.createElement(tag)
    for (var attr in attrs) {
      if (attr === 'style') {
        Object.assign(node.style, attrs[attr])
      } else {
        node.setAttribute(attr, attrs[attr])
      }
    }
    children && children.forEach(child => node.appendChild(typeof child === 'string' ? document.createTextNode(child) : child))
    return node
  }
  var named = tag => (attrs, children) => node(tag, attrs, children)
  var div = named('div')
  var span = named('span')
  var a = named('a')
  var raw = text => {
    var node = document.createElement('div')
    node.innerHTML = text
    return node
  };

  var render = (target, node) => {
    target.innerHTML = ''
    target.appendChild(node)
  };


const loadScript = absPath => {
  return new Promise((res, rej) => {
    const src = window.relativeToRoot + '/' + absPath;
    const script = node('script', {src});
    script.onload = () => res();
    script.onerror = e => rej(e);
    document.body.appendChild(script)
  })
}

const loadCss = absPath => {
  return new Promise((res, rej) => {
    const href = window.relativeToRoot + '/' + absPath;
    const link = node('link', {href, rel: 'stylesheet'});
    link.onload = () => res();
    link.onerror = e => rej(e);
    document.head.appendChild(link)
  })
}

const memoCss = absPath => {
  let promise = null
  return () => (promise || (promise = loadCss(absPath)))
};

const memoLoad = absPath => {
  let promise = null
  return () => (promise || (promise = loadScript(absPath)))
};

const loadDeps = memoLoad('all-deps.js');
const loadJsx = memoLoad('jsx-ppx.js');
const loadRefmt = memoLoad('refmt.js');
const loadOcaml = memoLoad('bucklescript.js');
const loadOcamlDeps = memoLoad('bucklescript-deps.js');
const loadCodeMirror = memoLoad('codemirror.js');
const loadRust = memoLoad('rust.js');
const loadSimple = memoLoad('simple.js');
const loadCodeMirrorCss = memoCss('codemirror.css');
const loadAll = () => Promise.all([loadJsx(), loadRefmt(), loadDeps(), loadOcaml().then(() => loadOcamlDeps())])

const runSandboxed = (script, logs, context) => {
  const addLog = (level, items) => {
    var text = ''
    if (items.length === 1 && typeof items[0] === 'string') {
      text = items[0]
    } else {
      text = JSON.stringify(items)
    }
    logs.appendChild(div({class: 'block-log level-' + level}, [text]))
  };

  const oldConsole = window.console
  window.console = Object.assign({}, window.console, {
    log: (...items) => {oldConsole.log(...items); addLog('log', items)},
    warn: (...items) => {oldConsole.warn(...items); addLog('warn', items)},
    error: (...items) => {oldConsole.error(...items); addLog('error', items)},
  });
  Object.assign(window, context)
  // ok folks we're done
  const exports = {}
  const module = {exports}
  const require = name => {
    return bsRequirePaths[name] ? window.packRequire(bsRequirePaths[name]) : window.packRequire(name)
  }
  try {
    // TODO check if it's a promise or something... and maybe wait?
    eval(script);
  } catch (error) {
    oldConsole.error(error)
    addLog('error', [error && error.message])
  }
  window.console = oldConsole
  for (let name in context) {
    window[name] = null
  }
};

var initBlocks = () => {
  ;[].forEach.call(document.querySelectorAll('div.block-target'), el => {
    const viewContext = el.getAttribute('data-context');
    const id = el.getAttribute('data-block-id');
    const parent = el.parentNode;

    const logs = div({class: 'block-logs'}, []);

    const bundleScript = document.querySelector('script[type=docre-bundle][data-block-id="' + id + '"]')
    const sourceScript = document.querySelector('script[type=docre-source][data-block-id="' + id + '"]')

    if (!bundleScript) {
      // not runnable, not editable
      return
    }

    let ran = false

    window.process = {env: {NODE_ENV: 'production'}}

    const runBlock = (context) => {
      if (ran) {
        return Promise.resolve()
      }
      ran = true
      return loadDeps().then(() => {
        console.log(id)
        if (!bundleScript) {
          console.error('bundle not found')
          return
        }
        runSandboxed(bundleScript.textContent, logs, context);
      })
    }
    let context = {}

    const betterShiftTab = /*onInfo => */cm => {
      var cursor = cm.getCursor()
        , line = cm.getLine(cursor.line)
        , pos = {line: cursor.line, ch: cursor.ch}
      // if (cursor.ch > 0 && line[cursor.ch - 1] !== ' ') {
      //   return cm.showHint({
      //     hint: onInfo,
      //     completeSingle: false,
      //     alignWithWord: false,
      //   })
      // }
      cm.execCommand('indentLess')
    }

    const betterTab = /*onComplete => */cm => {
      if (cm.somethingSelected()) {
        return cm.indentSelection("add");
      }
      const cursor = cm.getCursor()
      const line = cm.getLine(cursor.line)
      const pos = {line: cursor.line, ch: cursor.ch}
      // if (cursor.ch > 0 && line[cursor.ch - 1] !== ' ') {
      //   return cm.showHint({hint: onComplete})
      // }
      cm.replaceSelection(Array(cm.getOption("indentUnit") + 1).join(" "), "end", "+input");
    }


    const execute = (cm, code, before) => {
      logs.innerHTML = ''
      cm.getAllMarks().forEach(mark => {
        cm.removeLineWidget(mark)
      })
      const showError = (l1, c1, l2, c2) => {
        c2 = l1 === l2 && c1 === c2 ? c2 + 1 : c2;
        console.log('showing', l1, c1, l2, c2)
        cm.markText({line: l1 - before, ch: c1}, {line: l2 - before, ch: c2}, {
          className: 'CodeMirror-error-mark',
        })
      }
      return loadAll().then(() => {
        let ocaml
        try {
          ocaml = window.printML(window.parseRE(code))
        } catch (e) {
          if (e.location) {
            showError(e.location.startLine - 1, e.location.startLineStartChar - 1, e.location.endLine - 1, e.location.endLineEndChar - 1)
          }
          console.error(e)
          error.textContent = e.message
          error.style.display = 'block'
          return
        }
        let ppxed
        try {
          const {ppx_error_msg, js_error_msg, ocaml_code} = window.jsxv2.rewrite(ocaml)
          if (ppx_error_msg || js_error_msg) {
            console.error(ppx_error_msg, js_error_msg)
            error.textContent = (ppx_error_msg || '') + ' ' + (js_error_msg || '')
            error.style.display = 'block'
            return
          }
          ppxed = ocaml_code
        } catch (e) {
          console.error(e)
          error.textContent = e.message
          error.style.display = 'block'
          return
        }

        let js
        try {
          let result = window.ocaml.compile(ppxed)
          if (typeof result === 'string') {
            // bs 2.2.3
            result = JSON.parse(result)
          }
          const {js_code, js_error_msg, row, column, endRow, endColumn} = result
          if (!js_code && js_error_msg) {
            // TODO: just compile the straight reason, so these numbers mean something
            // showError(row, column, endRow, endColumn)
            error.textContent = js_error_msg
            error.style.display = 'block'
            console.log(result)
            return
          }
          js = js_code
        } catch (e) {
          console.error(e)
          error.textContent = e.message
          error.style.display = 'block'
          return
        }
        error.style.display = 'none'
        runSandboxed(js, logs, Object.assign({}, context))
      })
    }

    const pre = document.querySelector('pre.code[data-block-id="' + id + '"]')
    const error = div({class: 'code-block-error', style: {display: 'none'}})
    pre.insertAdjacentElement('afterend', error)

    const processHashes = code => {
      const lines = code.split('\n')
      let before = 0
      let after = 0
      for (let i=0; i<lines.length; i++) {
        let line = lines[i]
        if (line[0] == '#' || (line[0] == '!' && line[1] == '#')) {
          before = i + 1
        } else {
          break
        }
      }

      for (let i=1; i<=lines.length; i++) {
        let line = lines[lines.length - i]
        if (line[0] == '#' || (line[0] == '!' && line[1] == '#')) {
          after = i
        } else {
          break
        }
      }
      return {
        before,
        prefix: lines.slice(0, before).map(m => m.replace(/^!?#/, '')).join('\n') + (before > 0 ? '\n' : ''),
        mainCode: lines.slice(before, lines.length - after).join('\n'),
        suffix: (after > 0 ? '\n' : '') + lines.slice(lines.length - after).map(m => m.replace(/^!?#/, '')).join('\n'),
      }
    }

    let onEditRun = () => {};

    let loadingIcon = "⋯";
    let playIcon = "▶";

    let playButton
    if (viewContext === 'canvas') {
      playButton = div({class: 'block-canvas-play'}, ["▶"])
      const canvas = node('canvas', {id: 'block-canvas-' + id})
      canvas.width = 200
      canvas.height = 200
      context = {sandboxCanvas: canvas, sandboxCanvasId: canvas.id, containerDiv: parent}
      playButton.onclick = () => {
        playButton.textContent = loadingIcon
        runBlock(context).then(() => {
          playButton.style.display = 'none'
        })
      }
      const canvasBlock = div({class: 'block-canvas-container'}, [
        canvas,
        playButton
      ]);
      parent.appendChild(canvasBlock)
    } else if (viewContext === 'div') {
      const target = div({id: 'block-target-div-' + id})
      const container = div({class: 'block-target-container'}, [target])
      parent.appendChild(container)
      playButton = div({class: 'block-target-right'}, [playIcon])
      context = {sandboxDiv: target, sandboxDivId: target.id, containerDiv: parent}
      onEditRun = () => container.classList.add('active')
      playButton.onclick = () => {
        playButton.textContent = loadingIcon
        runBlock(context).then(() => {
          playButton.style.display = 'none'
          container.classList.add('active')
        })
      }
      parent.appendChild(playButton)
    } else {
      playButton = div({class: 'block-target-right'}, [playIcon])
      playButton.onclick = () => {
        playButton.textContent = loadingIcon
        runBlock({containerDiv: parent}).then(() => {
          playButton.style.display = 'none'
        })
      }
      parent.appendChild(playButton)
    }

    parent.appendChild(logs)

    if (!sourceScript) {
      // not editable
      return
    }
    const editButton = node('button', {class: 'code-edit-button'}, ["Edit"]);
    pre.appendChild(editButton)

    const startEditing = () => {
      editButton.textContent = loadingIcon
      if (playButton && playButton.parentNode) {
        playButton.parentNode.removeChild(playButton)
        playButton = null
      }
      // const code = sourceScript.textContent
      const {before, prefix, mainCode, suffix} = processHashes(sourceScript.textContent)
      console.log([prefix, mainCode, suffix])


      Promise.all([loadCodeMirror().then(() => loadSimple()).then(() => loadRust()), loadCodeMirrorCss()]).then(() => {
        const textarea = node('textarea', {class: 'code-block-editor', style: {width: '100%'}})
        pre.replaceWith(textarea)
        textarea.value = mainCode
        let playButton

        const run = (cm) => {
          onEditRun();
          playButton.textContent = loadingIcon
          execute(cm, prefix + cm.getValue() + suffix, before).then(() => {
            playButton.textContent = playIcon
          })
        }

        const cm = CodeMirror.fromTextArea(textarea, {
          lineNumbers: true,
          lineWrapping: true,
          viewportMargin: Infinity,
          extraKeys: {
            'Cmd-Enter': (cm) => run(cm),
            'Ctrl-Enter': (cm) => run(cm),
            Tab: betterTab,
            'Shift-Tab': betterShiftTab,
          },
          mode: 'rust',
        })

        playButton = node('button', {class: 'code-edit-run'}, ["▶"])
        playButton.onclick = () => run(cm)
        parent.appendChild(playButton)
      })
    }
    editButton.addEventListener('click', startEditing)

  })
}

window.addEventListener('load', () => {
  initBlocks();
})

})();