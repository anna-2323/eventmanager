import { api } from "../core/api.js";
import { $, $$, show } from "../core/dom.js";
import { header } from "../components/header.js";
import { loadSeatMap } from "../components/seatMap.js";

header();

const id = window.location.pathname.split("/").pop();

const e = await api.events.get(id);

// автоматично попълване на данни, ако потрвбителят е влязъл
const user = await api.auth.getUser();
if(user.logged_in) {
  $("#first_name").value = user.first_name;
  $("#last_name").value = user.last_name;
  $("#email").value = user.email;
  if(user.phone)
    $("#phone").value = user.phone;
}

$$("#event-title").textContent = e.title;

loadSeatMap(id);

$("#submit").addEventListener("click", async function () {
  const first_name = $("#first_name").value.trim();
  const last_name = $("#last_name").value.trim();
  const email = $("#email").value.trim();
  const phone = $("#phone").value.trim();
  const sector_id = parseInt($("#sector-id-input").dataset.sectorId);

  const error = $("#error");

  if (!first_name || !last_name || !email || !phone || !sector_id) {
    error.textContent = "Моля попълнете всички полета.";
    show(error);
    return;
  }

  const res = await api.tickets.purchase(id, 
    { sector_id, first_name, last_name, email, phone }
  );

  if (res.success) window.location.href = `/confirmation/${res.ticket_id}`;
  else {
    error.textContent = res.error || "Възникна грешка.";
    show(error);
  }
});
