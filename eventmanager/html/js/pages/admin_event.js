import { api } from "../core/api.js";
import { $, showSuccess, showError } from "../core/dom.js";
import { header } from "../components/header.js";
import { input, editSection, activateToggles } from "../components/form.js";
import { eventCard } from "../components/adminCards.js";
import { loadSeatMap } from "../components/seatMap.js";

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
  const event = await api.admin.events.get(id);
  renderEvent();
  renderLayout();

  showSuccess();
  showError();

  async function renderEvent() {
    $("#event-card").innerHTML = eventCard(event, user.role);
  }

  document.addEventListener("click", (e) => {
    if(e.target.matches("#layout-btn"))
      renderLayout();
    if(e.target.matches("#edit-btn"))
      renderEdit();
    if(e.target.matches("#activate-btn"))
      toggle_active();
    if(e.target.matches("#delete-btn"))
      renderDelete();

    // Редактиране на данни за събитие
    if(e.target.matches("#change-title-btn")) {
      const title = $("#new-title").value;
      editEvent({ title }, "Името е сменено успешно.");
    }
    if(e.target.matches("#change-begins-at-btn")) {
      const begins_at = $("#new-begins-at").value;
      editEvent({ begins_at }, "Времето е сменено успешно.");
    }
    if(e.target.matches("#change-description-btn")) {
      const description = $("#new-description").value;
      editEvent({ description }, "Описанието е сменено успешно.");
    }
    if(e.target.matches("#change-image-btn")) {
      const image = $("#new-image").files[0];
      const formData = new FormData();
      formData.append("image", image);
      editEventImage(formData, "Изображението е сменено успешно.");
    } 
  });

  function renderLayout() {
    $("#changeable").innerHTML = 
      `<div id="event-layout-field">
           <svg id="seat-map">
           </svg>
           <div id="sector-summary" style="visibility:hidden;">
             <div id="sector-id-input"></div>
             <p>Избран сектор: <strong id="summary-name"></strong></p>
             <p><span id="summary-price"></span><span id="summary-available"></span> останали места</p>
           </div>
       </div>`;
      loadSeatMap(id);
  }

  function renderEdit() {
    $("#changeable").innerHTML = `
        ${editSection(
          "Промяна на име",
          "change-title-form",
          `${input("Ново име", "new-title")}
          <button class="button is-link" id="change-title-btn">
              Запази
          </button>
        `,
        )}

      ${editSection(
        "Промяна на време",
        "change-begins-at-form",
        `${input("Ново време на започване", "new-begins-at", "datetime-local")}
        <button class="button is-link" id="change-begins-at-btn">
            Запази
        </button>
        `,
      )}

      ${editSection(
        "Промяна на описание",
        "change-description-form",
        `${input("Ново описание", "new-description",)}
        <button class="button is-link" id="change-description-btn">
            Запази
        </button>
        `,
      )}

      ${editSection(
        "Промяна на изображение",
        "change-image-form",
        `${input("Ново изображение", "new-image", "file")}
        <p class="help">JPG, PNG или WebP.</p>
        <button class="button is-link" id="change-image-btn">
            Запази
        </button>`
      )}`;

    activateToggles();
  }

  function toggle_active() {
    if(event.active) {
        editEvent(({ active: false }), "Събитието е успешно деактивирано.");
    }
    else {
        editEvent(({ active: true }), "Събитието е успешно активирано.");
    }
  }
  
  async function editEvent(json, message) {
    const res = await api.admin.events.edit(id, json);
    if (res.success) {
      localStorage.setItem("success_message", message);
      window.location.reload();
    } else {
      localStorage.setItem("error_message", res.error || "Възникна грешка.");
      window.location.reload();
    }
    window.location.reload();
  }

  async function editEventImage(data, message) {
    try {
        const res = await api.admin.events.editImage(id, data);
        if (res.success) {
          localStorage.setItem("success_message", message);
          window.location.reload();
        } else {
          $("#error").textContent = res.error || "Възникна грешка.";
          $("#error").style.display = "block";
        }
      } catch (err) {
        console.error(err);
        $("#error").textContent = "Възникна грешка.";
        $("#error").style.display = "block";
      }
  }
}
