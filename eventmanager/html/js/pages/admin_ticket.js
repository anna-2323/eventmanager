import { api } from "../core/api.js";
import { $, showError, showSuccess, editItem } from "../core/dom.js";
import { header } from "../components/header.js";
import { ticketCard } from "../components/adminCards.js";

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
  const { data: ticket } = await api.admin.tickets.get(id);
  renderTicket();
  
  showSuccess();
  showError();

   document.addEventListener("click", (e) => {
      if(e.target.matches("#activate-btn"))
        toggle_active();
   });

  async function renderTicket() {
    $("#ticket-card").innerHTML = ticketCard(ticket, user.role);
  }

  function toggle_active() {
    if(ticket.active)
      editItem(api.admin.tickets.edit, id, { active: false });
    else
      editItem(api.admin.tickets.edit, id, { active: true });
  }
}
