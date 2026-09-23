import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
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
}
else {
    // Запълване на менюто за избор на град
    const { data: cities } = await api.cities.list();
    $("#cities").innerHTML =
        `${cities.map((c) =>
            `<option value="${c}"></option>`
        ).join("")}`;

    $("#submit").addEventListener("click", async () => {
    const city = $("#city").value.trim();
    const address = $("#address").value.trim();
    const venue_name = $("#venue-name").value.trim();

    $("#error").style.display = "none";

    if (!city || !address || !venue_name) {
      $("#error").textContent = "Моля, попълнете всички полета.";
      $("#error").style.display = "block";
      return;
    }

    try {
      const res = await api.admin.venues.create({
        city,
        address,
        venue_name
      });

      if (res.success) {
        localStorage.setItem("success_message", res.message);
        window.location.href = "/admin/venues";
      } else {
        $("#error").textContent = res.message || "Възникна грешка.";
        $("#error").style.display = "block";
      }
    } catch (err) {
      console.error(err);
      $("#error").textContent = err.message || "Възникна грешка.";
      $("#error").style.display = "block";
    }
  });
}