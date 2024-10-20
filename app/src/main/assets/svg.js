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
        topIndexs = [15, 16, 18, 22, 20, 21, 2]
        bottomIndexs = [1, 23, 24, 25, 26, 27, 28]
    }
    insertItem(topIndexs, '.bar-renderer.top', 'bar-item-tab');
    insertItem(bottomIndexs, '.bar-renderer.bottom', 'bar-item-tab');
}
const synth = window.speechSynthesis;

const speak = (msg) => {
  let u = new SpeechSynthesisUtterance();
  u.lang = 'zh-TW';
  u.text = msg;
  synth.speak(u);
};
///////////////////////////////////////////////////
const items = [
    [
        1,
        "preview",
        "预览",
        async () => {
            await saveData();
            // if (typeof NativeAndroid !== 'undefined') {
            //     NativeAndroid.launchApp("psycho.euphoria.l", `/svgviewer?id=${id}`);
            // } else {
            //     window.open(`${baseUri}/svgviewer?id=${id}`, '_blank');
            // }
        }
    ], [
        2,
        "save",
        "保存",
        async () => {
            updateTags()
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
    ], [
        15,
        "preview",
        "保存",
        async () => {
            //await saveData();
            let line = getLine(textarea);
            NativeAndroid.speak(textarea.value.substring(line[0], line[1]));
        }
    ], [
        16,
        "image",
        "图片",
        async () => {
            await upload(baseUri);
        }
    ],
];

items.push([
    17,
    "download",
    "下载",
    async () => {
        await download(baseUri);
    }
]);
items.push([
    18,
    "g_translate",
    "翻译",
    async () => {
        await translateEnglish(textarea);
    }
]);
items.push([
    19,
    "translate",
    "翻译",
    async () => {
        await translateEnglish(textarea, 'en');
    }
]);
items.push([
    20,
    "image",
    "插入",
    async () => {
        await insertImage(baseUri);
    }
]);
items.push([
    21,
    "clear",
    "删除",
    () => {
        deleteBlock()
    }
]);
items.push([
    22,
    "title",
    "标题",
    () => {
        formatHead(textarea)
    }
]);
items.push([
    23,
    "format_bold",
    "粗体",
    () => {
        formatBold(textarea)
    }
]);
items.push([
    24,
    "format_align_center",
    "居中",
    () => {
        formatCenter(textarea)
    }
]);
items.push([
    25,
    "segment",
    "格式",
    () => {
        formatter(textarea)
    }
]);
items.push([
    26,
    "code",
    "代码",
    () => {
        formatCode(textarea)
    }
]);
items.push([
    27,
    "eco",
    "智能",
    () => {
        gemini(textarea)
    }
]);
items.push([
    28,
    "content_cut",
    "裁剪",
    () => {
        removeEnd();
    }
]);
document.addEventListener('keydown', async evt => {
    if (evt.ctrlKey) {
        if (evt.key === 's') {
            evt.preventDefault();
            await saveData();
        }
    } else {
        if (evt.key === 'F1') {
            evt.preventDefault();
            await translateEnglish(textarea);
        } else if (evt.key === 'F2') {
            evt.preventDefault();
            formatBold(textarea)
        } else if (evt.key === 'F3') {
            evt.preventDefault();
            deleteBlock()
            //textarea.value = "其他\n" + (await readText()).replace(`var createScene = `, `const createScene = async `)
        } else if (evt.key === 'F4') {
            evt.preventDefault();
            // await saveData();
            // if (typeof NativeAndroid !== 'undefined') {
            //     NativeAndroid.launchApp("psycho.euphoria.l", `/svgviewer?id=${id}`);
            // } else {
            //     window.open(`${baseUri}/svgviewer?id=${id}`, '_blank');
            // }
            formatHead(textarea)

        } else if (evt.key === 'F5') {
            evt.preventDefault();
            formatCode()
        } else if (evt.key === 'F6') {
            evt.preventDefault();
            functions(textarea);
        } else if (evt.key === 'F7') {
            evt.preventDefault();
            updateTags();
        }
    }

})
initializeToolbars()
loadData();