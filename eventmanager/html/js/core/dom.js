export function $(el, parent = document) {
    return parent.querySelector(el);
}

export function $$(el, parent = document) {
    return parent.querySelectorAll(el);
}

export function show(el) {
    el.style.display = 'block';
}

export function hide(el) {
    el.style.display = 'none';
}