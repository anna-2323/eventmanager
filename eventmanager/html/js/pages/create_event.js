import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
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
}
else {
    // Запълване на менюто за избор на зала
    const venues = await api.venues.list();

    $("#venue").innerHTML = `
      <option value="" disabled selected>Изберете зала</option>
      ${venues.map(v => `
        <option value="${v.id}">
          ${v.venue_name} — ${v.city}
        </option>
      `).join("")}
    `;

    $("#submit").addEventListener("click", async () => {
    const venue_id = Number($("#venue").value);
    const title = $("#title").value.trim();
    const begins_at = $("#begins-at").value.trim();
    const price = Number($("#price").value);
    const capacity = Number($("#capacity").value);

    $("#error").style.display = "none";

    if (!venue_id || !title || !begins_at || !price || !capacity) {
      $("#error").textContent = "Моля, попълнете всички полета.";
      $("#error").style.display = "block";
      return;
    }

    try {
      const res = await api.admin.events.create({
        venue_id,
        title,
        begins_at,
        price,
        capacity
      });

      if (res.success) {
        window.location.href = "/admin/events";
      } else {
        $("#error").textContent =
          res.error || "Възникна грешка.";
        $("#error").style.display = "block";
      }
    } catch (err) {
      console.error(err);
      $("#error").textContent = "Възникна грешка при добавянето.";
      $("#error").style.display = "block";
    }
  });
}