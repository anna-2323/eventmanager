import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const ROLE_LABELS = { 0: "Администратор", 1: "Организатор", 2: "Потребител" };

const user = await api.auth.getUser();
if (!user.logged_in || user.role !== 0) {
  $("#main").innerHTML =
    `<section class="section">
        <div class="container has-text-centered">
            <h1 class="title has-text-danger">Нямате права за достъп до тази страница</h1>
            <a class="button is-link" href="/home">Начало</a>
        </div>
    </section>`;
}

else {
  const users = await api.admin.users.list();
  $("#users-table").innerHTML = users.map((u) =>
    `<tr>
        <td>${u.id}</td>
        <td>${u.first_name} ${u.last_name}</td>
        <td>${u.email}</td>
        <td>
            <span class="tag ${u.role == 0 ? "is-danger" : u.role == 1 ? "is-warning" : "is-info"}">
                ${ROLE_LABELS[u.role] || u.role}
            </span>
        </td>
    </tr>`)
        .join("");
}
