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

export function toDate(d) {
  return new Date(d).toLocaleString("bg-BG", {
    day: "2-digit",
    month: "2-digit",
    year: "numeric",
  });
}

export function toDatetime(d) {
  return new Date(d).toLocaleString("bg-BG", {
    day: "2-digit",
    month: "2-digit",
    year: "numeric",
    hour: "2-digit",
    minute: "2-digit",
  });
}

export function toPrice(p) {
  return p.toLocaleString("bg-BG", { style: "currency", currency: "EUR" });
}
