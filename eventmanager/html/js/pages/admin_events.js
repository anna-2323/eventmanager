import { api } from "../core/api.js";
import { $, $$, toDate, showSuccess } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const { data: user } = await api.auth.getUser();
if (!user.logged_in || user.role == 2) {
  $("#main").innerHTML = `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
} else {
  showSuccess();
  $(".container").innerHTML = `
    <h1 class="title mt-4" id="table-title">Управление на събития</h1>
    <table class="table is-fullwidth is-striped is-hoverable">
        <thead>
            <tr>
                <th class="sortable" data-sort="title">
                  <span class="sortable-span">Име</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="begins-at">
                  <span class="sortable-span">Време</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="location">
                  <span>Локация</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="seats-left">
                  <span>Останали места</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="active">
                  <span>Активно</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th></th>
            </tr>
        </thead>
        <tbody id="events-table">
            <tr>
                <td colspan="4">Зареждане...</td>
            </tr>
        </tbody>
    </table>`

  const { data: events } = await api.admin.events.list();
  renderEvents();

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

      sortEvents();
      renderEvents();
    });
  });

  function sortEvents() {
    const { field, ascending } = currentSort;

    events.sort((a, b) => {
      let av, bv;

      switch (field) {
        case "title":
          av = a.title.replace(" ", "").toLowerCase();
          bv = b.title.replace(" ", "").toLowerCase();
          break;

        case "begins-at":
          av = a.begins_at.replace(" ", "T");
          bv = b.begins_at.replace(" ", "T");
          break;

        case "location":
          av = `${a.venue_name}, ${a.city}`.toLowerCase();
          bv = `${b.venue_name}, ${b.city}`.toLowerCase();
          break;

        case "seats-left":
          av = a.seats_left;
          bv = b.seats_left;
          break;

        case "active":
          av = a.active;
          bv = b.active;
          break;

      }

      if (av < bv) return ascending ? -1 : 1;
      if (av > bv) return ascending ? 1 : -1;
      return 0;
    });
  }
  
  function renderEvents() {
    $("#events-table").innerHTML = events
      .map(
        (e) =>
          `<tr>
              <td>${e.title}</td>
              <td>${toDate(e.begins_at)}</td>
              <td>${e.venue_name}, ${e.city}</td>
              <td>${e.seats_left}</td>
              <td>${e.active ? '<i class="fa-solid fa-check"></i>' : ''}
              <td>
                  <a href="/admin/events/${e.id}"><button class="button is-link view-btn">Преглед</button></a>
              </td>
            </tr>`,
      )
      .join("");
  }
}
