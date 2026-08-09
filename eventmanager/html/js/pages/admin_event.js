import { api } from "../core/api.js";
import { $, $$, toDate } from "../core/dom.js";
import { header } from "../components/header.js";
import { input, editSection, activateToggles } from "../components/form.js";
import { eventCard } from "../components/adminCards.js";
import { loadSeatMap } from "../components/seatMap.js";

header();

const user = await api.auth.getUser();
if (!user.logged_in || user.role !== 0) {
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

  async function renderEvent() {
    $("#event-card").innerHTML = eventCard(event);
  }

  document.addEventListener("click", (e) => {
    // Смяна на изглед
    if(e.target.matches("#layout-btn"))
      renderLayout();
    if(e.target.matches("#edit-btn"))
      renderEdit();
    if(e.target.matches("#verify-btn"))
      verify();
    if(e.target.matches("#delete-btn"))
      renderDelete();

    // Редактиране на данни за събитие
    if(e.target.matches("#change-title-btn")) {
      const title = $("#new-title").value;
      editEvent({ title }, "Името е сменено успешно.");
    }
    if(e.target.matches("#change-time-btn")) {
      const time = $("#new-time").value;
      editEvent({ time }, "Времето е сменено успешно.");
    }
  });

  function renderLayout() {
    $("#changeable").innerHTML = 
      `<div id="event-layout-field" style="display:none;">
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
        "change-time-form",
        `${input("Ново време на започване", "new-time", "datetime-local")}
        <button class="button is-link" id="change-time-btn">
            Запази
        </button>
        `,
      )}
        `;

    activateToggles();
  }

  function verify() {
    if(event.verified) {
        editEvent(({ verified: false }), "Одобряването на събитието е отменено.");
    }
    else {
        editEvent(({ verified: true }), "Събитието е одобрено.");
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

  $("#confirm-delete-btn").addEventListener("click", async function () {
      const res = await api.admin.events.delete(id);
      if (res.success) {
        localStorage.setItem("success_message", "Успешно изтрито събитие.");
        window.location.href = `/admin/events`;
      } else {
        localStorage.setItem("error_message", res.error || "Възникна грешка.");
      }
  });
  
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
}
