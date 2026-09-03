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

export function showSuccess() {
  const success_message = localStorage.getItem("success_message");
  if (success_message) {
    const el = $("#success-message");
    el.textContent = success_message;
    show(el);
    localStorage.removeItem("success_message");
  }
}

export function showError() {
  const error_message = localStorage.getItem("error_message");
  if (error_message) {
    const el = $("#error-message");
    el.textContent = error_message;
    show(el);
    localStorage.removeItem("error_message");
  }
}