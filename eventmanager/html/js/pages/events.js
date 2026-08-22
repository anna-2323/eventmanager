import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";
import { getEventCard } from "../components/eventCard.js";

await header();

let events = [];

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

let currentPage = 1;
const pageSize = 18;

// Първоначални филтри, взети от URL 
const urlParams = new URLSearchParams(window.location.search);

const initialSearch = urlParams.get("search") || "";
currentCategory = urlParams.get("category");
currentCity = urlParams.get("city");

$("#events-search").value = initialSearch;

updateSearchSubtitle(initialSearch);

// Показват се текущо избраните филтри
if (currentCategory) {
  const item = document.querySelector(
    `#category-filter .dropdown-item[data-value="${CSS.escape(currentCategory)}"]`,
  );

  if (item) {
    $("#category-filter .dropdown-trigger button span:first-child")
      .textContent = item.textContent.trim();
  }
}
if (currentCity) {
  const item = document.querySelector(
    `#city-filter .dropdown-item[data-value="${CSS.escape(currentCity)}"]`,
  );

  if (item) {
    $("#city-filter .dropdown-trigger button span:first-child")
      .textContent = item.textContent.trim();
  }
}

// Зареждане на събития
async function loadEvents() {
  const params = {};

  const search = $("#events-search").value.trim();

  if (search) {
    params.search = search;
  }

  if (currentCategory) {
    params.category = currentCategory;
  }

  if (currentCity) {
    params.city = currentCity;
  }

  events = await api.events.list(params);

  sortEvents();
  renderEvents();
}

// Показване на събития
function renderEvents() {
  const start = (currentPage - 1) * pageSize;
  const end = start + pageSize;

  const pageEvents = events.slice(start, end);

  $("#events").innerHTML = pageEvents
    .map((e) => getEventCard(e))
    .join("");

  renderPagination();
}

// Pagination
function renderPagination() {
  const totalPages = Math.ceil(events.length / pageSize);

  if (totalPages <= 1) {
    $("#pagination").innerHTML = "";
    return;
  }

  const pages = [];

  // Първа стр.
  pages.push(1);

  // Многоточие преди текуща страница
  if (currentPage > 3) {
    pages.push("ellipsis");
  }

  // Страниците около текущата
  for (
    let page = Math.max(2, currentPage - 1);
    page <= Math.min(totalPages - 1, currentPage + 1);
    page++
  ) {
    pages.push(page);
  }

  // Многоточие след текуща страница
  if (currentPage < totalPages - 2) {
    pages.push("ellipsis");
  }

  // Последна стр.
  if (totalPages > 1) {
    pages.push(totalPages);
  }

  let html = `
    <nav class="pagination is-centered"
    >
      <button
        class="pagination-previous"
        data-page="${currentPage - 1}"
        ${currentPage === 1 ? "disabled" : ""}
      >
        Предишна
      </button>

      <button
        class="pagination-next"
        data-page="${currentPage + 1}"
        ${currentPage === totalPages ? "disabled" : ""}
      >
        Следваща
      </button>

      <ul class="pagination-list">
  `;

  for (const page of pages) {
    if (page === "ellipsis") {
      html += `
        <li>
          <span class="pagination-ellipsis">&hellip;</span>
        </li>
      `;

      continue;
    }

    html += `
      <li>
        <button
          class="pagination-link ${page === currentPage ? "is-current" : ""}"
          data-page="${page}" 
          ${page === currentPage ? 'aria-current="page"' : ""}
        >
          ${page}
        </button>
      </li>
    `;
  }

  html += `
      </ul>
    </nav>
  `;

  $("#pagination").innerHTML = html;
}

// Dropdown
document.addEventListener("click", (e) => {
  const item = e.target.closest(".dropdown-item");

  if (!item) {
    return;
  }

  // Сортиране
  if (item.dataset.sort) {
    currentSort = item.dataset.sort;
    currentPage = 1;

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

    $("#city-filter .dropdown-trigger button span:first-child")
      .textContent = item.textContent.trim();

    return;
  }
});

// Pagination страници
document.addEventListener("click", (e) => {
  const button = e.target.closest(".pagination-link");

  if (!button) {
    return;
  }

  const page = Number(button.dataset.page);
  const totalPages = Math.ceil(events.length / pageSize);

  if (!page || page < 1 || page > totalPages) {
    return;
  }

  currentPage = page;

  renderEvents();

  $("#events").scrollIntoView({
    behavior: "smooth",
    block: "start",
  });
});

// Предишен/следващ pagination бутони
document.addEventListener("click", (e) => {
  const button = e.target.closest(
    ".pagination-previous, .pagination-next",
  );

  if (!button || button.disabled) {
    return;
  }

  const page = Number(button.dataset.page);
  const totalPages = Math.ceil(events.length / pageSize);

  if (!page || page < 1 || page > totalPages) {
    return;
  }

  currentPage = page;

  renderEvents();

  $("#events").scrollIntoView({
    behavior: "smooth",
    block: "start",
  });
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

  updateSearchSubtitle(search);

  // Отиване на първа страница при смяна на филтри
  currentPage = 1;

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

      default:
        return 0;
    }
  });
}

// Помощна функция за показване на търсене
function updateSearchSubtitle(search) {
  if (search) {
    $("#events-subtitle").textContent =
      `Резултати за '${search}'`;
  } else {
    $("#events-subtitle").textContent = "";
  }
}

// Начално зареждане
await loadEvents();
