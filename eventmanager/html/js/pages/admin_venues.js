import { api } from "../core/api.js";
import { $, $$, toDate } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const user = await api.auth.getUser();
if (!user.logged_in || user.role == 2) {
  $("#main").innerHTML = `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
} else {
  $(".container").innerHTML = `
    <h1 class="title mt-4" id="table-title">Управление на зали</h1>
    <table class="table is-fullwidth is-striped is-hoverable">
        <thead>
            <tr>
                <th class="sortable" data-sort="id">
                  <span class="sortable-span">ID</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="venue-name">
                  <span class="sortable-span">Име</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="city">
                  <span class="sortable-span">Град</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="address">
                  <span class="sortable-span">Адрес</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                <th></th>
            </tr>
        </thead>
        <tbody id="venues-table">
            <tr>
                <td colspan="4">Зареждане...</td>
            </tr>
        </tbody>
    </table>`

  const venues = await api.venues.list();
  renderVenues();

  let currentSort = {
    field: null,
    ascending: true,
  };

  const clickableArray = [...$$(".sortable")];
  const iconArray = [...$$(".change-icon")];

  clickableArray.forEach((th, i) => {
    th.addEventListener("click", function () {
      const field = th.dataset.sort;

      if (currentSort.field === field)
        currentSort.ascending = !currentSort.ascending;
      else {
        currentSort.field = field;
        currentSort.ascending = true;
      }

      // Премахване на другите икони
      iconArray.forEach((icon) => {
        icon.classList.remove("fa-chevron-up", "fa-chevron-down");
      });

      // Добавяне на правилната икона
      iconArray[i].classList.add(
        currentSort.ascending ? "fa-chevron-up" : "fa-chevron-down",
      );

      sortVenues();
      renderVenues();
    });
  });

  function sortVenues() {
    const { field, ascending } = currentSort;

    venues.sort((a, b) => {
      let av, bv;

      switch (field) {
        case "id":
          av = a.id;
          bv = b.id;
          break;

        case "venue-name":
          av = a.venue_name.replace(" ", "").toLowerCase();
          bv = b.venue_name.replace(" ", "").toLowerCase();
          break;

        case "city":
          av = a.city.replace(" ", "").toLowerCase();
          bv = b.city.replace(" ", "").toLowerCase();
          break;

        case "address":
          av = a.address.replace(" ", "").toLowerCase();
          bv = b.address.replace(" ", "").toLowerCase();
          break;
      }

      if (av < bv) return ascending ? -1 : 1;
      if (av > bv) return ascending ? 1 : -1;
      return 0;
    });
  }
  
  function renderVenues() {
    $("#venues-table").innerHTML = venues
      .map(
        (v) =>
          `<td>${v.id}</td>
              <td>${v.venue_name}</td>
              <td>${v.city}</td>
              <td>${v.address}</td>
              <td>
                  <a href="/admin/venues/${v.id}"><button class="button is-link view-btn">Преглед</button></a>
              </td>
            </tr>`,
      )
      .join("");
  }
}
