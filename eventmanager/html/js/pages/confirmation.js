import { api } from "../core/api.js";
import { $, toDate, toPrice } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const ticket_id = window.location.pathname.split("/").pop();

const { data: t } = await api.tickets.confirm(ticket_id);
$("#confirmation").innerHTML = `<h1 class="title">Успешна покупка!</h1>
    <p><strong>${t.event_name}</strong></p>
    <p><strong>${toPrice(t.price)}</strong></p>
    <p>Дата: <i>${toDate(t.begins_at)}</i></p>
    <p>Адрес: <i>${t.venue_name}, ${t.venue_address}</i></p>
    ${t.sector ? `<p>Сектор: <i>${t.sector}</i></p>` : ""}
    <p>Имена: <i>${t.first_name} ${t.last_name}</i></p>
    <a class="button is-link mt-4" href="/events">Към всички събития</a>`;
