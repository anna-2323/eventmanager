import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";
import { getEventCard } from "../components/eventCard.js";

header();

const params = new URLSearchParams(window.location.search);

// search bar
if (params.get("search") && params.get("search").length > 0) {
  $('.navbar input[type="search"]').value = params.get("search");
  $("#events-subtitle").innerHTML =
    `Резултати за '${params.get("search")}'`;
}

const events = await api.events.list(params);
$("#events").innerHTML = events.map((e) => getEventCard(e)).join("");
