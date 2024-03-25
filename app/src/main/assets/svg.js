

let baseUri = window.location.host === "127.0.0.1:5500" ? "http://192.168.8.55:8090" : "..";
const searchParams = new URL(window.location).searchParams;
let id = searchParams.get('id');
const path = searchParams.get('path');
const textarea = document.getElementById("textarea");

function insertItem(indexs, selector, klass) {
    const bottomBar = document.querySelector(selector);
    if (!bottomBar) return;
    bottomBar.innerHTML = '';
    const array = [];
    for (let j = 0; j < indexs.length; j++) {
        for (let index = 0; index < items.length; index++) {
            if (indexs[j] === items[index][0]) {
                array.push(items[index]);
                break;
            }
        }
    }
    array.filter(x => indexs.indexOf(x[0]) !== -1).forEach(x => {
        const div = document.createElement('div');
        div.className = klass || "item";
        div.innerHTML = `<span class="material-symbols-outlined">${x[1]}</span>
        <div class="pivot-bar-item-title">${x[2]}</div>`;
        div.addEventListener('click', evt => {
            evt.stopPropagation();
            x[3]();
        });
        bottomBar.appendChild(div);
    })
}
async function saveData() {
    let s = textarea.value.trim();
    let nid = id ? parseInt(id, 10) : 0;
    let body = {
        id: nid,
        title: substringBefore(s, "\n").trim(),
        content: substringAfter(s, "\n").trim()
    };
    // await submitNote(getBaseUri(), JSON.stringify(body));
    // document.getElementById('toast').setAttribute('message', '成功');
    let res;
    try {
        res = await fetch(`${baseUri}/svg`, {
            method: 'POST',
            body: JSON.stringify(body)
        });
        if (res.status !== 200) {
            throw new Error();
        }
        if (!nid) {
            var queryParams = new URLSearchParams(window.location.search);
            let sid = await res.text();
            queryParams.set("id", sid);
            id = parseInt(sid);
            history.replaceState(null, null, "?" + queryParams.toString());
        }
        toast.setAttribute('message', '成功');
    } catch (error) {
        toast.setAttribute('message', '错误');
    }
}

async function loadData() {
    const res = await fetch(`${baseUri}/svg?id=${id}`, { cache: "no-store" });
    const body = await res.json();;
    document.title = body["title"];
    textarea.value = `${body["title"]}
    
${body["content"]}`;
}

function formatCode() {
    const options = { indent_size: 2 }
    if (textarea.value.indexOf("const createScene = ") !== -1) {
        textarea.value = js_beautify(textarea.value, options);
    } else {
        textarea.value = html_beautify(textarea.value, options);
    }
}
function getWord(textarea) {
    let start = textarea.selectionStart;
    let end = textarea.selectionEnd;
    while (start - 1 > -1 && /[a-zA-Z0-9_\u3400-\u9FBF.]/.test(textarea.value[start - 1])) {
        start--;
    }
    while (end < textarea.value.length && /[a-zA-Z0-9_\u3400-\u9FBF.]/.test(textarea.value[end])) {
        end++;
    }
    return [start, end];
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
async function snippet(textarea) {
    const points = getWord(textarea);
    let word = textarea.value.substring(points[0], points[1]);

    if (word.indexOf('.') !== -1) {
        let start = substringBefore(word, ".");
        let end = substringAfter(word, ".");
        if (end === 'not') {
            textarea.setRangeText(`if(${start}!==0){
                }else{
                }`, points[0], points[1]);
            return;
        } else {
            textarea.setRangeText(`let ${start} = 0;`, points[0], points[1]);
            return;
        }
    }
    const res = await fetch(`${baseUri}/snippet?k=${word}&t=0`, { cache: "no-store" })
    if (res.status === 200) {
        textarea.setRangeText(await res.text(), points[0], points[1]);
    }
}
function commentLine(textarea) {
    const points = findExtendPosition(textarea);
    let s = textarea.value.substring(points[0], points[1]).trim();
    if (textarea.value[textarea.selectionStart] === '<') {
        if (s.startsWith("<!--") && s.endsWith("-->")) {
            s = s.substring("<!--".length);
            s = s.substring(0, s.length - "-->".length);
        } else {
            s = `<!--${s}-->`;
        }
    }
    else {
        if (s.startsWith("/*") && s.endsWith("*/")) {
            s = s.substring("/*".length);
            s = s.substring(0, s.length - "*/".length);
        } else {
            s = `/*${s}*/`;
        }
    }
    textarea.setRangeText(s, points[0], points[1]);
}
function animateShape(selectedString) {
    let svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    svg.innerHTML = selectedString;
    svg.style.position = 'absolute';
    svg.style.left = '-100%';
    document.body.appendChild(svg);

    var len = svg.children[0].getTotalLength();
    svg.remove();
    return `
${substringBefore(selectedString, '>')} 
fill="none" 
stroke="red" 
stroke-width="4"
stroke-dasharray="${len}"
stroke-dashoffset="${len}" >
<animate id="" 
begin="1s" 
attributeName="stroke-dashoffset" 
values="${len};0" 
dur="1s" 
calcMode="linear" 
fill="freeze"/>
</${selectedString.match(/(?<=<)[^ ]+/)}>
`;
}
function animatePath(selectedString) {
    let svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    var path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
    path.setAttribute('d', selectedString);
    svg.appendChild(path);
    svg.style.position = 'absolute';
    svg.style.left = '-100%';
    document.body.appendChild(svg);

    var len = path.getTotalLength();
    svg.remove();
    return `
<path d="${selectedString}" 
fill="none" 
stroke="red" 
stroke-width="4"
stroke-dasharray="${len}"
stroke-dashoffset="${len}" >
<animate id="" 
begin="1s" 
attributeName="stroke-dashoffset" 
values="${len};0" 
dur="1s" 
calcMode="linear" 
fill="freeze"/>
</path>
`;
}
function drawPolygon(x, y, n, radius, options = {}) {
    const array = [];
    array.push(
        [x + radius * Math.cos(options.rotation || 0),
        y + radius * Math.sin(options.rotation || 0)]);
    for (let i = 1; i <= n; i += 1) {
        const angle = (i * (2 * Math.PI / n)) + (options.rotation || 0);
        array.push(
            [
                x + radius * Math.cos(angle),
                y + radius * Math.sin(angle)
            ])
    }
    const d = `M${array.map(x => `${x[0]} ${x[1]}`).join('L')}`;
    return `<path stroke="#FF5722" fill="none" stroke-width="4"  d="${d}">
</path>`
}
function getTotalLength(d) {
    let svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    var path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
    path.setAttribute('d', d);
    svg.appendChild(path);
    svg.style.position = 'absolute';
    svg.style.left = '-100%';
    document.body.appendChild(svg);

    var len = path.getTotalLength();
    svg.remove();
    return len;
}
function getBBox(s, isLen) {
    let svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    svg.innerHTML = s;
    svg.style.position = 'absolute';
    svg.style.left = '-100%';
    document.body.appendChild(svg);

    var len = isLen ? svg.children[0].getTotalLength() : svg.children[0].getBBox();
    svg.remove()
    return len;
}
function getCenterPath(s, offset) {
    let svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    svg.innerHTML = s;
    svg.style.position = 'absolute';
    svg.style.left = '-100%';
    document.body.appendChild(svg);

    var len = svg.children[0].getTotalLength();
    let first = svg.children[0].getPointAtLength(len / 2 - offset)
    let second = svg.children[0].getPointAtLength(len / 2 + offset)

    svg.remove()
    return [first, second];
}
function getNormalPath(s, offset) {
    let svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    svg.innerHTML = s;
    svg.style.position = 'absolute';
    svg.style.left = '-100%';
    document.body.appendChild(svg);

    var len = svg.children[0].getTotalLength();
    let firstPoint = svg.children[0].getPointAtLength(0)
    let secondPoint = svg.children[0].getPointAtLength(len)
    var deg = Math.atan2(firstPoint.y - secondPoint.y, firstPoint.x - secondPoint.x) * (180 / Math.PI);
    deg = 180 - deg;
    var offsetX = Math.sin(deg / Math.PI * 180) * offset;
    var offsetY = Math.cos(deg / Math.PI * 180) * offset;

    svg.remove()
    return `M${firstPoint.x + offsetX} ${firstPoint.y + offsetY}L${secondPoint.x + offsetX} ${secondPoint.y + offsetY}`;
}
function calculateMoveAlongNormalPath(s) {
    let offset = substringBefore(s, "<path").trim();
    let path = "<path" + substringAfter(s, "<path");
    let v = getNormalPath(path, parseInt(offset))
    return s.replace(/(?<=d=")[^"]+(?=")/, v);
}
function calculateCenterTextPath(s) {
    let path = substringBefore(s, "<text");
    let text = "<text" + substringAfter(s, "<text");
    let box = getBBox(text, false);
    const points = getCenterPath(path, box.width / 2);
    const first = points[0];
    const second = points[1];
    let v = `M${first.x} ${first.y}L${second.x} ${second.y}`
    return `<defs>
    <path
      id="tp1"
      fill="none"
      stroke="red"
      d="${v}" />
    </defs>
  
  

    <text font-size="36px" font-family="苹方">
      <textPath href="#tp1" startOffset="-100%">${s.match(/(?<=>)[^<]+(?=<\/text)/)}
      <animate attributeName="startOffset" from="-100%" to ="0%" begin="0s" dur="1s" repeatCount="1" id="t1" fill="freeze"/>
      </textPath>
    </text>
    
    `

    //s.replace(/(?<=d=")[^"]+(?=")/, v);
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
function calculate() {

    processSelection(s => {
        if (s.indexOf('<path') !== -1 &&
            s.indexOf('<text') !== -1) {
            return calculateCenterTextPath(s)
        } else if (/^[0-9]+<path/.test(s)) {
            return calculateMoveAlongNormalPath(s)
        } else if (/\d+,\d+,\d+,\d+/.test(s)) {
            return eval(`drawPolygon(${s})`);
        } else if (/^d="[^"]+"/.test(s)) {
            return animatePath(/(?<=d=")[^"]+/.exec(s)[0])
        } else if (s.startsWith("<") && s.endsWith(">")) {
            return animateShape(s)
        } else {
            return eval(s);
        }
    })
}

function searchWord(textarea) {

    let selectedString = textarea.value.substring(textarea.selectionStart, textarea.selectionEnd).trim();
    s = selectedString;

    let index = textarea.value.indexOf(s, textarea.selectionEnd);
    if (index === -1)
        index = textarea.value.indexOf(s);

    textarea.focus();
    textarea.scrollTop = 0;
    const fullText = textarea.value;
    textarea.value = fullText.substring(0, index + s.length);
    textarea.scrollTop = textarea.scrollHeight;
    textarea.value = fullText;

    textarea.selectionStart = index;
    textarea.selectionEnd = index + s.length;

}


function getLineBackforward(textarea, start) {
    let end = start;
    while (start - 1 > -1) {
        if (textarea.value[start] === '\n') break;
        start--;
    }
    return [start, end];
}
function getBlockString() {
    let selectionStart = textarea.selectionStart;
    let count = 0;
    // while (selectionStart - 1 > -1) {
    //     if (textarea.value[selectionStart] === '{') {
    //         if (count == 0)
    //             break;
    //         else count--;
    //     } else if (textarea.value[selectionStart] === '}') {
    //         count++;
    //     }
    //     selectionStart--;
    // }
    while (selectionStart - 1 > -1) {
        if (textarea.value[selectionStart] === '{') {
            let points = getLineBackforward(textarea, selectionStart);
            let s = textarea.value.substring(points[0], points[1]);
            if (/\(\s*\)/.test(s)) {
                selectionStart = points[0];
                break;
            }
        }
        selectionStart--;
    }
    let selectionEnd = textarea.selectionEnd;
    while (selectionEnd < textarea.value.length) {
        if (textarea.value[selectionEnd] === '}') {
            if (count == 0)
                break;
            else count--;
        } else if (textarea.value[selectionEnd] === '{') {
            count++;
        }
        selectionEnd++;
    }
    return [selectionStart, selectionEnd];

}

async function functions(textarea) {

    let points = getWord(textarea);
    let s = textarea.value.substring(points[0], points[1]).trim();
    let t = 'zh-CN';
    if (/[\u3400-\u9FBF]/.test(s)) {
        t = 'en'
    }
    let name = "f";
    try {
        const response = await fetch(`${baseUri}/trans?to=${t}&q=${encodeURIComponent(s)}`);
        if (response.status > 399 || response.status < 200) {
            throw new Error(`${response.status}: ${response.statusText}`)
        }
        const results = await response.json();
        const trans = results.sentences.map(x => x.trans);
        name = camel(trans.join(' '));

    } catch (error) {
        console.log(error);
    }


    points = findExtendPosition(textarea);
    s = substringAfter(textarea.value.substring(points[0], points[1]).trim(), "\n");
    let rvm = /(?<=const |var )[a-z][a-zA-Z0-9_]*?(?= )/.exec(s);
    let rv = (rvm && rvm[0]) || "v"

    let vvm = [...new Set([...s.matchAll(/(?<=[ \(])[a-z][a-zA-Z0-9_]*(?=[\),])/g)].map(x => x[0]))]
    let vsm = [...new Set([...s.matchAll(/(?<=const |var )[a-z][a-zA-Z0-9_]*(?=\S)/g)].map(x => x[0]))]
    vsm.push(...["true", "false"])
    let array = [];
    console.log(vvm,vsm);
    for (let i = 0; i < vvm.length; i++) {
        if (vsm.indexOf(vvm[i]) === -1) {
            array.push(vvm[i]);
        }
    }
    vvm = array;

    let ssPoints = getBlockString(textarea);
    textarea.setRangeText(`const ${rv} = ${name}(${vvm.join(", ")});`, points[0], points[1]);
    let selectionStart = ssPoints[0];
    s = `function ${name}(${vvm.join(", ")}){
${s}
return ${rv};
}
`
    textarea.setRangeText(s, selectionStart, selectionStart);
}

function findReplace(textarea) {

    let points = getLine(textarea);
    let first = textarea.value.substring(points[0], points[1]).trim();;
    let p = getLineAt(textarea, points[1] + 1);
    let line = textarea.value.substring(p[0], p[1]).trim()
    if (line.indexOf("{") !== -1) {
        console.log(line);
        let end = points[0];
        let count = 0;
        while (end < textarea.value.length) {
            end++;
            if (textarea.value[end] === '{') {
                count++;
            } else if (textarea.value[end] === '}') {
                count--;
                if (count === 0) {
                    end++;
                    break;
                }
            }
        }
        let s = textarea.value.substring(points[1], end);
        const pieces = first.split(/ +/);
        console.log(s.replaceAll(new RegExp("\\b" + pieces[0] + "\\b", 'g')))
        textarea.setRangeText(s.replaceAll(new RegExp("\\b" + pieces[0] + "\\b", 'g'), pieces[1]), points[1], end);

    }

    else {


        const points = findExtendPosition(textarea);
        let s = textarea.value.substring(points[0], points[1]).trim();
        const first = substringBefore(s, "\n");
        const second = substringAfter(s, "\n");
        const pieces = first.split(/ +/);
        s = `${first}  
${second.replaceAll(new RegExp("\\b" + pieces[0] + "\\b", 'g'), pieces[1])}`;
        textarea.setRangeText(s, points[0], points[1]);
    }

}
async function translate(textarea) {

    let points = getWord(textarea);
    let s = textarea.value.substring(points[0], points[1]).trim();
    let t = 'zh-CN';
    if (/[\u3400-\u9FBF]/.test(s)) {
        t = 'en'
    }
    try {
        const response = await fetch(`${baseUri}/trans?to=${t}&q=${encodeURIComponent(s)}`);
        if (response.status > 399 || response.status < 200) {
            throw new Error(`${response.status}: ${response.statusText}`)
        }
        const results = await response.json();
        const trans = results.sentences.map(x => x.trans);
        let name = camel(trans.join(' '));
        textarea.setRangeText(`const ${name} = "0";`, points[0], points[1]);
    } catch (error) {
        console.log(error);
    }


}
function deleteLine(textarea) {
    const points = getLine(textarea);
    let s = textarea.value.substring(points[0], points[1]).trim();
    writeText(s);
    textarea.setRangeText("", points[0], points[1]);
}
function copyLine(textarea) {
    if (textarea.value[textarea.selectionStart] === "<") {
        let start = textarea.selectionStart;
        let end = start;
        while (end < textarea.value.length && /[<a-z0-9A-Z]/.test(textarea.value[end])) {
            end++;
        }
        let str = textarea.value.substring(start + 1, end);

        while (end < textarea.value.length) {
            if (textarea.value[end] === "/" && textarea.value.substring(end + 1, end + 1 + str.length) === str) {
                end += str.length + 3;
                break;
            }
            end++;
        }
        str = textarea.value.substring(start, end);
        textarea.setRangeText(str, end + 1, end + 1);
        return
    }
    let points = findExtendPosition(textarea);
    let s = textarea.value.substring(points[0], points[1]).trim();


    let str = `
${s.replace(/\b[a-zA-Z_]+[0-9]+\b/g, v => {
        let vv = /([a-zA-Z_]+)([0-9]+)/.exec(v);
        return vv[1] + (parseInt(vv[2]) + 1);
    })}`;
    let selectionEnd = textarea.selectionEnd;

    while (selectionEnd < textarea.value.length && textarea.value[selectionEnd] !== '\n') {
        selectionEnd++;
    }
    textarea.setRangeText(str, selectionEnd, selectionEnd);

}
async function initializeToolbars() {
    let topIndexs;
    let bottomIndexs;

    try {
        const response = await fetch(`${baseUri}/ts`);
        if (response.status > 399 || response.status < 200) {
            throw new Error(`${response.status}: ${response.statusText}`)
        }
        const results = JSON.parse(await response.text());
        if (results) {
            topIndexs = results[0];
            bottomIndexs = results[1];
        }
    } catch (error) {
        topIndexs = [1, 6, 7, 8, 4, 11, 12]
        bottomIndexs = [2, 3, 5, 9, 10, 13, 14]
    }
    insertItem(topIndexs, '.bar-renderer.top', 'bar-item-tab');
    insertItem(bottomIndexs, '.bar-renderer.bottom', 'bar-item-tab');
}
///////////////////////////////////////////////////
const items = [
    [
        1,
        "preview",
        "预览",
        async () => {
            await saveData();
            if (typeof NativeAndroid !== 'undefined') {
                NativeAndroid.launchApp("psycho.euphoria.l", `/svgviewer?id=${id}`);
            } else {
                window.open(`${baseUri}/svgviewer?id=${id}`, '_blank');
            }
        }
    ], [
        2,
        "save",
        "保存",
        async () => {
            saveData();
        }
    ], [
        3,
        "code",
        "格式",
        () => {
            formatCode();
        }
    ], [
        4,
        "help",
        "帮助",
        () => {
            window.open(`${baseUri}/snippet.html`, '_blank');
        }
    ],
    [
        5,
        "text_snippet",
        "代码",
        () => {
            snippet(textarea);
        }
    ], [
        6,
        "text_snippet",
        "计算",
        () => {
            let points = getLine(textarea);
            let line = textarea.value.substring(points[0], points[1]);
            let results = eval(line);
            writeText(results);
            textarea.setRangeText(`${line}
            ${results}
            `, points[0], points[1])
        }
    ], [
        7,
        "comment",
        "注释",
        () => {
            commentLine(textarea)
        }
    ], [
        8,
        "animation",
        "动画",
        () => {
            calculate()
        }
    ], [
        9,
        "search",
        "搜索",
        () => {
            searchWord(textarea);
        }
    ], [
        10,
        "text_snippet",
        "函数",
        () => {
            functions(textarea);
        }
    ], [
        11,
        "find_replace",
        "替换",
        () => {
            findReplace(textarea);
        }
    ], [
        12,
        "text_snippet",
        "翻译",
        () => {
            translate(textarea);
        }
    ], [
        13,
        "close",
        "剪行",
        () => {

            deleteLine(textarea)
        }
    ], [
        14,
        "content_copy",
        "复行",
        () => {
            copyLine(textarea)
        }
    ],
];
document.addEventListener('keydown', async evt => {
    if (evt.ctrlKey) {
        if (evt.key === 's') {
            evt.preventDefault();
            await saveData();
        }
    } else {
        if (evt.key === 'F1') {
            evt.preventDefault();
            snippet(textarea);
        } else if (evt.key === 'F2') {
            evt.preventDefault();
            location = '/svg.html'
        } else if (evt.key === 'F3') {
            evt.preventDefault();
            textarea.value = "其他\n" + (await readText()).replace(`var createScene = `, `const createScene = async `)
        } else if (evt.key === 'F4') {
            evt.preventDefault();
            await saveData();
            if (typeof NativeAndroid !== 'undefined') {
                NativeAndroid.launchApp("psycho.euphoria.l", `/svgviewer?id=${id}`);
            } else {
                window.open(`${baseUri}/svgviewer?id=${id}`, '_blank');
            }
        }
    }

})
initializeToolbars()
loadData();