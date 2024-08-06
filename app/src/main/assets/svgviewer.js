const headers = [...document.querySelectorAll('h2,h3')];
const buffer = [];
headers.forEach((element, index) => {
    element.id = `header-${index}`;
    buffer.push({ id: `header-${index}`, value: element.textContent.trim() });
});
const array = [];
// href="#${element.id}"
buffer.forEach(element => {
    array.push(`<li><a>${element.value}</a></li>`);
});
const html = `<ul class="toc">
${array.join('\n')}
</ul>`

document.querySelector('.container').insertAdjacentHTML('beforebegin', html);