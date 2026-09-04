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
    <h1 class="title mt-4" id="table-title">Управление на билети</h1>
    <table class="table is-fullwidth is-striped is-hoverable">
        <thead>
            <tr>
                <th class="sortable" data-sort="id">
                  <span class="sortable-span">ID</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="names">
                  <span class="sortable-span">Име</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="title">
                  <span class="sortable-span">Събитие</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="location">
                  <span class="sortable-span">Зала</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="sector">
                  <span>Сектор</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="active">
                  <span>Активен</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th></th>
            </tr>
        </thead>
        <tbody id="tickets-table">
            <tr>
                <td colspan="4">Зареждане...</td>
            </tr>
        </tbody>
    </table>`

  const tickets = await api.admin.tickets.list();
  renderTickets();

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

      sortTickets();
      renderTickets();
    });
  });

  function sortTickets() {
    const { field, ascending } = currentSort;

    tickets.sort((a, b) => {
      let av, bv;

      switch (field) {
        case "id":
          av = a.id;
          bv = b.id;
          break;

        case "names":
          av = `${a.first_name} ${a.last_name}`.toLowerCase();
          bv = `${b.first_name} ${b.last_name}`.toLowerCase();
          break;

        case "title":
          av = a.event_name.replace(" ", "").toLowerCase();
          bv = b.event_name.replace(" ", "").toLowerCase();
          break;

        case "location":
          av = `${a.venue_name}, ${a.city}`.toLowerCase();
          bv = `${b.venue_name}, ${b.city}`.toLowerCase();
          break;

        case "sector":
          av = a.sector_name.toLowerCase();
          bv = b.sector_name.toLowerCase();
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
  
  function renderTickets() {
    if(tickets.length > 0)
        $("#tickets-table").innerHTML = tickets
          .map(
            (t) =>
              `<td>${t.id}</td>
                  <td>${t.first_name} ${t.last_name}</td>
                  <td>${t.event_name}</td>
                  <td>${t.venue_name}, ${t.venue_city}</td>
                  <td>${t.sector_name ? t.sector_name : ""}</td>
                  <td>${t.active ? '<i class="fa-solid fa-check"></i>' : ''}
                  <td>
                      <a href="/admin/tickets/${t.id}"><button class="button is-link view-btn">Преглед</button></a>
                  </td>
                </tr>`,
          )
          .join("");
    else {
        $("#tickets-table").innerHTML = "";
    }
  }
}
