
class CustomSelect extends HTMLElement {

    constructor() {
        super();
        this.attachShadow({
            mode: 'open'
        });
        const wrapper = document.createElement("div");
        wrapper.setAttribute("class", "wrapper");
        const style = document.createElement('style');
        style.textContent = `.btn {
    font-weight: 500;
    box-sizing: border-box;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 0 16px;
    height: 36px;
    font-size: 14px;
    line-height: 36px;
    border-radius: 18px;
  }
  
  .dialog-buttons {
    flex-shrink: 0;
    display: flex;
    -webkit-box-align: center;
    align-items: center;
    -webkit-box-pack: end;
    justify-content: flex-end;
    margin-top: 12px
  }
  
  .dialog-body {
    overflow-y: auto;
    overflow-x: auto;
    max-height: 50vh;
    /*white-space: pre-wrap*/
  }
  
  .modern-overlay {
    position: fixed;
    top: 0;
    bottom: 0;
    left: 0;
    right: 0;
    z-index: 1;
    cursor: pointer;
    background-color: rgba(0, 0, 0, 0.3)
  }
  
  h2 {
    display: -webkit-box;
    -webkit-box-orient: vertical;
    max-height: 2.5em;
    -webkit-line-clamp: 2;
    overflow: hidden;
    line-height: 1.25;
    text-overflow: ellipsis;
    font-weight: normal;
    font-size: 1.8rem
  }
  
  .dialog-header {
    display: flex;
    -webkit-box-orient: vertical;
    -webkit-box-direction: normal;
    flex-direction: column;
    flex-shrink: 0
  }
  
  .dialog {
    position: relative;
    z-index: 2;
    display: flex;
    -webkit-box-orient: vertical;
    -webkit-box-direction: normal;
    flex-direction: column;
    max-height: 100%;
    box-sizing: border-box;
    padding: 16px;
    margin: 0 auto;
    overflow-x: hidden;
    overflow-y: auto;
    font-size: 1.3rem;
    color: #0f0f0f;
    border: none;
    min-width: 250px;
    max-width: 356px;
    box-shadow: 0 0 24px 12px rgba(0, 0, 0, 0.25);
    border-radius: 12px;
    background-color: #fff
  }
  
  .dialog-container {
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    display: flex;
    -webkit-box-align: center;
    align-items: center;
    -webkit-box-pack: center;
    justify-content: center;
    z-index: 4;
    margin: 0 40px;
    padding: 0 env(safe-area-inset-right) env(safe-area-inset-bottom) env(safe-area-inset-left)
  }
  
  `;
        this.wrapper = wrapper;
        this.shadowRoot.append(style, wrapper);
    }




    connectedCallback() {
        this.wrapper.innerHTML = `<div class="dialog-container">
    <div class="dialog">
      <div class="dialog-header">
        <h2 bind="header">${this.getAttribute("title") || '代码'}</h2>
      </div>
      <div class="dialog-body">
        <slot></slot>
      </div>
      <div class="dialog-buttons">
        <div class="btn close">
          删除
        </div>
        <div class="btn submit" style="color: #909090">
          导入
        </div>
      </div>
    </div>
    <div  class="modern-overlay close">
    </div>
  </div>`;
        this.wrapper.querySelectorAll('.close')
            .forEach(element => {
                element.addEventListener('click', () => {
                    this.remove();

                })
            });
        this.wrapper.querySelector('.submit').addEventListener('click', () => {
            this.remove();
            this.dispatchEvent(new CustomEvent('submit'));
        })
        this.wrapper.querySelector('.btn.close').addEventListener('click', () => {
            this.dispatchEvent(new CustomEvent('close'));
        })
    }

    static get observedAttributes() {
        return ['title'];
    }

    attributeChangedCallback(attrName, oldVal, newVal) {

    }
}

customElements.define('custom-select', CustomSelect);

/*
绑定元素和事件
例如：<div bind="div" @click="click"></div>
执行下列代码后，即可通过 this.div 访问该元素
在全局下编写click函数，即可自动绑定到该元素的click事件
*/
function bind(elememnt) {
    (elememnt || document).querySelectorAll('[bind]').forEach(element => {
        if (element.getAttribute('bind')) {
            window[element.getAttribute('bind')] = element;
        }
        [...element.attributes].filter(attr => attr.nodeName.startsWith('@')).forEach(attr => {
            if (!attr.value) return;
            element.addEventListener(attr.nodeName.slice(1), evt => {
                if (window[attr.value])
                    window[attr.value](evt);
                else {
                    window['actions'][attr.value](evt);
                }
            });
        });
    })
}

function camel(string) {
    let s = string.replaceAll(/[ _-]([a-zA-Z])/g, m => m[1].toUpperCase());
    return s.slice(0, 1).toLowerCase() + s.slice(1);
}
function findString() {
    let start = textarea.selectionStart
    let end = textarea.selectionEnd;
    while (start - 1 > -1 && textarea.value[start - 1] != '`') {
        start--;
    }
    while (end + 1 < textarea.value.length && textarea.value[end] != '`') { end++; }
    return textarea.value.substring(start, end);
}
function collectElements() {

    console.log(JSON.stringify([...document.querySelectorAll(".bar-renderer [id]")]
        .map((x, i) => {
            const obj = {
                id: i + 1,
                d: x.querySelector('path').getAttribute('d'),
                title: x.querySelector('.bar-item-title').textContent.trim(),
                attr: x.id
            };
            return obj;
        })));

}

function humanFileSize(size) {
    if (size === 0) return '0';
    var i = Math.floor(Math.log(size) / Math.log(1024));
    return (size / Math.pow(1024, i)).toFixed(2) * 1 + ' ' + ['B', 'kB', 'MB', 'GB', 'TB'][i]
}
function getLine(textarea) {
    let start = textarea.selectionStart;
    let end = textarea.selectionEnd;
    if (textarea.value[start] === '\n' && start - 1 > 0) {
        start--;
    }
    if (textarea.value[end] === '\n' && end - 1 > 0) {
        end--;
    }
    while (start - 1 > -1 && textarea.value[start - 1] !== '\n') {
        start--;
    }
    while (end + 1 < textarea.value.length && textarea.value[end + 1] !== '\n') {
        end++;
    }
    return [start, end + 1];
}
function getLineAt(textarea, start) {


    if (textarea.value[start] === '\n' && start + 1 < textarea.value.length) {
        start++;
    }
    let end = start;
    while (start - 1 > -1 && textarea.value[start - 1] !== '\n') {
        start--;
    }
    while (end + 1 < textarea.value.length && textarea.value[end + 1] !== '\n') {
        end++;
    }
    return [start, end + 1];
}
function getStatement(textarea) {
    let start = textarea.selectionStart;
    let end = textarea.selectionEnd;
    if (textarea.value[start] === '\n' && start - 1 > 0) {
        start--;
    }
    if (textarea.value[end] === '\n' && end - 1 > 0) {
        end--;
    }
    while (start - 1 > -1 && textarea.value[start - 1] !== '\n') {
        start--;
    }
    while (end + 1 < textarea.value.length && textarea.value[end + 1] !== ';') {
        end++;
    }
    return [start, end + 1];
}
function getWord(textarea) {
    let start = textarea.selectionStart;
    let end = textarea.selectionEnd;
    while (start - 1 > -1 && /[a-zA-Z0-9_\u3400-\u9FBF]/.test(textarea.value[start - 1])) {
        start--;
    }
    while (end < textarea.value.length && /[a-zA-Z0-9_\u3400-\u9FBF]/.test(textarea.value[end])) {
        end++;
    }
    return [start, end];
}
function getNumber(textarea) {
    let start = textarea.selectionStart;
    let end = textarea.selectionEnd;
    while (start - 1 > -1 && /[0-9.]/.test(textarea.value[start - 1])) {
        start--;
    }
    while (end + 1 < textarea.value.length && /[0-9.]/.test(textarea.value[end])) {
        end++;
    }
    return [start, end];
}
function getWordString(textarea) {
    let start = textarea.selectionStart;
    let end = textarea.selectionEnd;
    while (start - 1 > -1 && /[a-zA-Z0-9.]/.test(textarea.value[start - 1])) {
        start--;
    }
    while (end + 1 < textarea.value.length && /[a-zA-Z0-9.]/.test(textarea.value[end])) {
        end++;
    }
    return [start, end];
}
function findExtendPosition(editor) {
    const start = editor.selectionStart;
    const end = editor.selectionEnd;
    let string = editor.value;
    let offsetStart = start;
    while (offsetStart > 0) {
        if (!/\s/.test(string[offsetStart - 1]))
            offsetStart--;
        else {
            let os = offsetStart;
            while (os > 0 && /\s/.test(string[os - 1])) {
                os--;
            }
            if ([...string.substring(offsetStart, os).matchAll(/\n/g)].length > 1) {
                break;
            }
            offsetStart = os;
        }
    }
    let offsetEnd = end;
    while (offsetEnd < string.length) {
        if (!/\s/.test(string[offsetEnd + 1])) {

            offsetEnd++;
        } else {

            let oe = offsetEnd;
            while (oe < string.length && /\s/.test(string[oe + 1])) {
                oe++;
            }
            if ([...string.substring(offsetEnd, oe + 1).matchAll(/\n/g)].length > 1) {
                offsetEnd++;

                break;
            }
            offsetEnd = oe + 1;

        }
    }
    while (offsetStart > 0 && string[offsetStart - 1] !== '\n') {
        offsetStart--;
    }
    // if (/\s/.test(string[offsetEnd])) {
    //     offsetEnd--;
    // }
    return [offsetStart, offsetEnd];
}

function writeText(message) {
    const textarea = document.createElement("textarea");
    textarea.style.position = 'fixed';
    textarea.style.right = '100%';
    document.body.appendChild(textarea);
    textarea.value = message;
    textarea.select();
    document.execCommand('copy');
    textarea.remove();
}

async function readText() {
    // const textarea = document.createElement("textarea");
    // textarea.style.position = 'fixed';
    // textarea.style.right = '100%';
    // document.body.appendChild(textarea);
    // textarea.value = message;
    // textarea.select();
    // document.execCommand('paste');
    // return textarea.value;
    let strings;
    if (typeof NativeAndroid !== 'undefined') {
        strings = NativeAndroid.readText()
    } else {
        strings = await navigator.clipboard.readText()
    }
    return strings
}
function changeInputMethod() {
    if (typeof NativeAndroid !== 'undefined') {
        NativeAndroid.launchInputPicker()
    }
}

function findNextLineStart() {
    let selectionEnd = textarea.selectionEnd;
    while (selectionEnd < textarea.value.length) {
        selectionEnd++;
        if (textarea.value[selectionEnd] === '\n') {
            selectionEnd++;
            break;
        }
    }
    return selectionEnd;
}
function snake(string) {
    return string.replaceAll(/(?<=[a-z])[A-Z]/g, m => `_${m}`).toLowerCase()
        .replaceAll(/[ -]([a-z])/g, m => `_${m[1]}`)
}

function substring(strings, prefix, suffix) {
    let start = strings.indexOf(prefix);
    if (start === -1) {
        return [0, 0]
    }
    start += prefix.length;
    let end = strings.indexOf(suffix, start);
    if (end === -1) {
        return [0, 0]
    }
    return [start, end]
}

function substringAfter(string, delimiter, missingDelimiterValue) {
    const index = string.indexOf(delimiter);
    if (index === -1) {
        return missingDelimiterValue || string;
    } else {
        return string.substring(index + delimiter.length);
    }
}

function substringAfterLast(string, delimiter, missingDelimiterValue) {
    const index = string.lastIndexOf(delimiter);
    if (index === -1) {
        return missingDelimiterValue || string;
    } else {
        return string.substring(index + delimiter.length);
    }
}

function substringBefore(string, delimiter, missingDelimiterValue) {
    const index = string.indexOf(delimiter);
    if (index === -1) {
        return missingDelimiterValue || string;
    } else {
        return string.substring(0, index);
    }
}

function substringBeforeLast(string, delimiter, missingDelimiterValue) {
    const index = string.lastIndexOf(delimiter);
    if (index === -1) {
        return missingDelimiterValue || string;
    } else {
        return string.substring(0, index);
    }
}

function substringNearest(string, index, start, end) {
    let j = index;
    while (j > -1) {
        if (start.indexOf(string[j]) !== -1) {
            j++
            break;
        }
        j--;
    }
    let k = index;
    while (k < string.length) {
        if (end.indexOf(string[k]) !== -1) {
            break;
        }
        k++;
    }
    return string.substring(j, k);
}
function substringAll(strings, prefix, suffix, fn) {
    let offset = 0;
    while (true) {
        let start = strings.indexOf(prefix, offset);
        if (start === -1) {
            return strings;
        }
        start += prefix.length;
        let end = strings.indexOf(suffix, start + prefix.length);
        if (end === -1) {
            return strings;
        }
        let s = fn(strings.substring(start, end));
        strings = strings.substring(0, start) + s + strings.substring(end);
        offset += start + s.length + suffix.length;
    }
    return strings;
}

function upperCamel(string) {
    string = camel(string);
    return string.slice(0, 1).toUpperCase() + string.slice(1);
}

function delay(time) {
    return new Promise(resolve => setTimeout(resolve, time));
}

function processSelection(fn) {
    let selectionStart = textarea.selectionStart;
    let selectionEnd = textarea.selectionEnd;
    let selectedString = textarea.value.substring(selectionStart, selectionEnd);
    if (!selectedString) {
        selectedString = getLine(textarea);
        if (textarea.value[selectionStart] !== '\n') {
            while (selectionStart + 1 < textarea.value.length && textarea.value[selectionStart + 1] !== '\n') {
                selectionStart++;
            }
            selectionStart++;
        }

        selectionEnd = selectionStart
        textarea.value = `${textarea.value.substring(0, selectionStart)}
${fn(selectedString.trim())}${textarea.value.substring(selectionEnd)}`;
        return;
    }
    textarea.value = `${textarea.value.substring(0, selectionStart)}${fn(selectedString.trim())}${textarea.value.substring(selectionEnd)}`;

}

function escapeHtml(unsafe) {
    return unsafe
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

