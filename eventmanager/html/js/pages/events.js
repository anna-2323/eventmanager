import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";
import { getEventCard } from "../components/eventCard.js";

await header();

const params = new URLSearchParams(window.location.search);

// Попълване на dropdown менютата
const cities = await api.cities.list();
const categories = await api.categories.list();
$('#city-dropdown').innerHTML = cities.map(city =>
  
  `<a class="dropdown-item" data-value="${city}">${city}</a>`
).join("");
$("#category-dropdown").innerHTML = categories.map(c => 
  `<a class="dropdown-item" data-value="${c.id}">${c.title}</a>`
).join("");

let currentSort = null;
let currentCategory = null;
let currentCity = null;

// Получаване на филтри от URL
const initialSearch = params.get("search") || "";
currentCategory = params.get("category");
currentCity = params.get("city");

$("#events-search").value = initialSearch;

// Показват се текущо избраните филтри
if (currentCategory) {
  const item = document.querySelector(
    `#category-filter .dropdown-item[data-value="${CSS.escape(currentCategory)}"]`,
  );

  if (item) {
    $(
      "#category-filter .dropdown-trigger button span:first-child",
    ).textContent = item.textContent;
  }
}

if (currentCity) {
  const item = document.querySelector(
    `#city-filter .dropdown-item[data-value="${CSS.escape(currentCity)}"]`,
  );

  if (item) {
    $("#city-filter .dropdown-trigger button span:first-child").textContent =
      item.textContent;
  }
}

// Надпис за търсене
if (initialSearch.length > 0) {
  $("#events-subtitle").textContent = `Резултати за '${initialSearch}'`;
}

// Зареждане на събития
let events = [];

async function loadEvents() {
  const params = {
    search: $("#events-search").value.trim(),
    category: currentCategory,
    city: currentCity,
  };

  events = await api.events.list(params);

  sortEvents();
  renderEvents();
}

function renderEvents() {
  $("#events").innerHTML = events.map((e) => getEventCard(e)).join("");
}

// Dropdown
document.addEventListener("click", (e) => {
  const item = e.target.closest(".dropdown-item");

  if (!item) {
    return;
  }

  // Сортиране
  if (item.dataset.sort) {
    const sort = item.dataset.sort;

    if (currentSort !== sort) {
      currentSort = sort;
    }

    sortEvents();
    renderEvents();

    return;
  }

  // Категории
  if (item.closest("#category-filter")) {
    currentCategory = item.dataset.value;

    $(
      "#category-filter .dropdown-trigger button span:first-child",
    ).textContent = item.textContent.trim();

    return;
  }

  // Градове
  if (item.closest("#city-filter")) {
    currentCity = item.dataset.value;

    $("#city-filter .dropdown-trigger button span:first-child").textContent =
      item.textContent.trim();

    return;
  }
});

// Прилагане на филтри
$("#btn-filters-apply").addEventListener("click", async () => {
  const search = $("#events-search").value.trim();

  // Обновява се URL
  const params = new URLSearchParams();

  if (search) {
    params.set("search", search);
  }

  if (currentCategory) {
    params.set("category", currentCategory);
  }

  if (currentCity) {
    params.set("city", currentCity);
  }

  const queryString = params.toString();

  const newUrl = queryString
    ? `${window.location.pathname}?${queryString}`
    : window.location.pathname;

  window.history.pushState({}, "", newUrl);

  // Обновява се текстът с търсене
  if (search) {
    $("#events-subtitle").textContent = `Резултати за '${search}'`;
  } else {
    $("#events-subtitle").innerHTML = "";
  }

  await loadEvents();
});

// Изчистване на филтри
$("#btn-filters-clear").addEventListener("click", async () => {
    // Изчистване на стойностите
    currentCategory = null;
    currentCity = null;
    $("#events-search").value = "";

    // Връщане на оригиналните надписи
    $(
      "#category-filter .dropdown-trigger button span:first-child",
    ).textContent = "Категория";

    $("#city-filter .dropdown-trigger button span:first-child").textContent =
      "Град";

    // Изчистване на subtitle
    $("#events-subtitle").textContent = "";

    // Изчистване на URL
    window.history.pushState({}, "", window.location.pathname);

    // Изчистване на сортиране
    currentSort = "upcoming";
    sortEvents();

    await loadEvents();
});

// Сортиране
function sortEvents() {
  events.sort((a, b) => {
    let av, bv;

    switch (currentSort) {
      case "upcoming":
        av = a.begins_at.replace(" ", "T");
        bv = b.begins_at.replace(" ", "T");

        if (av === bv) return 0;
        return av > bv ? 1 : -1;

      case "price_asc":
        av = a.price;
        bv = b.price;

        if (av == bv) return 0;
        return av >= bv ? 1 : -1;

      case "price_desc":
        av = a.price;
        bv = b.price;

        if (av == bv) return 0;
        return av >= bv ? -1 : 1;
    }

    return 0;
  });
}

// Начално зареждане
await loadEvents();
