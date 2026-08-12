import { api } from "../core/api.js";
import { $, $$, toDate } from "../core/dom.js";
import { header } from "../components/header.js";
import { input, editSection, activateToggles } from "../components/form.js";
import { venueCard } from "../components/adminCards.js";

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
  const id = window.location.pathname.split("/").pop();
  const venue = await api.venues.get(id);
  renderVenue();
  renderEvents();

  async function renderVenue() {
    $("#venue-card").innerHTML = venueCard(venue);
  }

  document.addEventListener("click", (e) => {
    // Смяна на изглед
    if(e.target.matches("#events-btn"))
      renderEvents();
    if(e.target.matches("#edit-btn"))
      renderEdit();
    if(e.target.matches("#verify-btn"))
      verify();
    if(e.target.matches("#delete-btn"))
      renderDelete();

    // Редактиране на данни за събитие
    if(e.target.matches("#change-name-btn")) {
      const title = $("#new-name").value;
      editVenue({ title }, "Името е сменено успешно.");
    }
    if(e.target.matches("#confirm-delete-btn")) {
      if(venue.active)
        editVenue({ active: false }, "Залата е успешно деактивирана.");
      else
        editVenue({ active: true }, "Залата е успешно активирана.");
    }
  });

  async function renderEvents() {
    const events = await api.venues.getEvents(id);

    $("#changeable").innerHTML = `<h2 id="events-title" class="subtitle">
            ${"Провеждани събития"}
        </h2>`;

    if (events.length > 0) {
      $("#changeable").innerHTML += `
        <table class="table is-fullwidth is-striped is-hoverable" id="events-table">
          <tr> 
            <th>Име</th>
            <th>Време</th>
            <th></th>
          </tr>
        </table>`
      $("#events-table").innerHTML += events
        .map(
          (e) =>
            `<tr>
                  <td>${e.title}</td>
                  <td>${toDate(e.begins_at)}</td>
                  <td><a href="/admin/events/${e.id}"><button class="button is-small is-link">
                          Преглед
                      </button></a></td>
              </tr>`,
        ).join("");
    } else {
      $("#changeable").innerHTML += "<p>Все още няма събития.</p>";
    }
  }

  function renderEdit() {
    $("#changeable").innerHTML = `
        ${editSection(
          "Промяна на име",
          "change-name-form",
          `${input("Ново име", "new-name")}
          <button class="button is-link" id="change-name-btn">
              Запази
          </button>
        `,
        )}
        `;

    activateToggles();
  }

  function verify() {
    if(venue.verified) {
        editVenue(({ verified: false }), "Одобряването на събитието е отменено.");
    }
    else {
        editVenue(({ verified: true }), "Събитието е одобрено.");
    }
  }
  // Модал
  function openModal(modal) {
  if (!modal.classList.contains("is-active")) modal.classList.add("is-active");
  }

  function closeModal(modal) {
    if (modal.classList.contains("is-active"))
      modal.classList.remove("is-active");
  }

  function renderDelete() {
    openModal($("#delete-modal"));
  }

  ($$(".modal-background, #modal-close, #modal-cancel") || []).forEach((x) => {
    x.addEventListener("click", () => {
      closeModal(x.closest(".modal"));
    });
  });

  $().addEventListener("click", async function () {
      const res = await api.admin.events.delete(id);
      if (res.success) {
        localStorage.setItem("success_message", "Успешно изтрито събитие.");
        window.location.href = `/admin/venues`;
      } else {
        localStorage.setItem("error_message", res.error || "Възникна грешка.");
      }
  });
  
  async function editVenue(json, message) {
    const res = await api.admin.venues.edit(id, json);
    if (res.success) {
      localStorage.setItem("success_message", message);
      window.location.reload();
    } else {
      localStorage.setItem("error_message", res.error || "Възникна грешка.");
      window.location.reload();
    }
    window.location.reload();
  }
}
