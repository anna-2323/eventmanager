import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";

header();

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
  const events = await api.admin.events.list();
  $("#events-table").innerHTML = events.map((e) =>
    `<tr>
        <td>${e.id}</td>
        <td>${e.title}</td>
        <td>${new Date(e.begins_at).toLocaleString("bg-BG")}</td>
        <td>${e.venue_name}, ${e.city}</td>
        <td>${e.seats_left}</td>
    </tr>`)
        .join("");
}
