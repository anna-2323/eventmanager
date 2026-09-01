import { api } from "../core/api.js";
import { $, $$ } from "../core/dom.js";
import { header } from "../components/header.js";
import { ticketCard } from "../components/adminCards.js";

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
  const ticket = await api.admin.tickets.get(id);
  renderTicket();

  async function renderTicket() {
    $("#ticket-card").innerHTML = ticketCard(ticket, user.role);
  }

  document.addEventListener("click", (e) => {
    if(e.target.matches("#delete-btn"))
      renderDelete();
  });

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
}
