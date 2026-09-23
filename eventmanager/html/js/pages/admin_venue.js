import { api } from "../core/api.js";
import { $, showError, showSuccess, toDate, editItem } from "../core/dom.js";
import { header } from "../components/header.js";
import { input, editSection, activateToggles } from "../components/form.js";
import { venueCard } from "../components/adminCards.js";

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
  const id = window.location.pathname.split("/").pop();
  const { data: venue } = await api.venues.get(id);
  renderVenue();
  renderEvents();
  
  showSuccess();
  showError();

  async function renderVenue() {
    $("#venue-card").innerHTML = venueCard(venue, user.role);
  }

  document.addEventListener("click", (e) => {
    if(e.target.matches("#events-btn"))
      renderEvents();
    if(e.target.matches("#edit-btn"))
      renderEdit();
    if(user.role == 0 && e.target.matches("#activate-btn"))
      set_active();

    // Редактиране на данни за събитие
    if(e.target.matches("#change-name-btn")) {
      const venue_name = $("#new-name").value;
      if(venue_name)
        editItem(api.admin.venues.edit, id, { venue_name });
      else {
        localStorage.setItem("error_message", "Моля, въведете ново име на залата.");
        window.location.reload();
      }
    }
    if(e.target.matches("#change-address-btn")) {
      const address = $("#new-address").value;
      if(address)
        editItem(api.admin.venues.edit, id, { address });
      else {
        localStorage.setItem("error_message", "Моля, въведете нов адрес.");
        showError();
      }
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
        ${editSection(
          "Промяна на адрес",
          "change-address-form",
          `${input("Нов адрес", "new-address")}
          <button class="button is-link" id="change-address-btn">
              Запази
          </button>
        `,
        )}
        `;

    activateToggles();
  }

  function set_active() {
    if(venue.active) {
        editItem(api.admin.venues.edit, id, { active: false });
    }
    else {
       editItem(api.admin.venues.edit, id, { active: true });
    }
  }
}
