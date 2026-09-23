import { api } from "../core/api.js";
import { $, $$, showSuccess } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const ROLE_LABELS = { 0: "Администратор", 1: "Организатор", 2: "Потребител" };

const { data: user } = await api.auth.getUser();
if (!user.logged_in || user.role !== 0) {
  $("#main").innerHTML = `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
} else {
  showSuccess();
  $(".container").innerHTML = `
    <h1 class="title mt-4">Управление на потребители</h1>
    <table class="table is-fullwidth is-striped is-hoverable">
        <thead>
            <tr>
                <th class="sortable" data-sort="name">
                  <span class="sortable-span">Име</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="email">
                  <span class="sortable-span">Имейл</span>
                  <span class="icon">
                    <i class="change-icon fas"></i>
                  </span>
                </th>
                <th class="sortable" data-sort="role">
                  <span>Роля</span>
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
                <th>Планирано изтриване</th>
                <th></th>
            </tr>
        </thead>
        <tbody id="users-table">
            <tr>
                <td colspan="4">Зареждане...</td>
            </tr>
        </tbody>
    </table>`;

  let { data: users } = await api.admin.users.list();
  renderUsers();

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
        iconArray.forEach(icon => {
            icon.classList.remove("fa-chevron-up", "fa-chevron-down");
        });

        // Добавяне на правилната икона
        iconArray[i].classList.add(
            currentSort.ascending
                ? "fa-chevron-up"
                : "fa-chevron-down"
        );

      sortUsers();
      renderUsers();
    });
  });

  function sortUsers() {
    const { field, ascending } = currentSort;

    users.sort((a, b) => {
      let av, bv;

      switch (field) {
        case "name":
          av = `${a.first_name} ${a.last_name}`.toLowerCase();
          bv = `${b.first_name} ${b.last_name}`.toLowerCase();
          break;

        case "email":
          av = a.email.toLowerCase();
          bv = b.email.toLowerCase();
          break;

        case "role":
          av = a.role;
          bv = b.role;
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

  function renderUsers() {
    $("#users-table").innerHTML = users
        .map((u) =>
            `<tr ${u.deleted_on ? 'class="to-delete"' : ""}>
            <td>${u.first_name} ${u.last_name}</td>
            <td>${u.email}</td>
            <td>
                <span class="tag ${u.role == 0 ? "is-danger" : u.role == 1 ? "is-warning" : "is-info"}">
                    ${ROLE_LABELS[u.role] || u.role}
                </span>
            </td>
            <td>${u.active ? '<i class="fa-solid fa-check"></i>' : ''}
            <td>
                ${u.deleted_on ? `${new Date(u.deleted_on).toLocaleString("bg-BG")}` : ""}
            </td>
            <td>
                <a href="/admin/users/${u.id}"><button class="button is-link view-btn">Преглед</button></a>
            </td>
        </tr>`,
        )
        .join("");
  }
}
