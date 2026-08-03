import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";
import { getEventCard } from "../components/eventCard.js";

header();
// TODO: да се извеждат най-популярните събития
const events = await api.events.list();
$(".splide__list").innerHTML = events
  .map(
    (e) => `
    <li class="splide__slide">
        ${getEventCard(e)}
    </li>
`,
  )
  .join("");

new Splide(".splide", {
  type: "loop",
  perPage: 3,
  gap: "1rem",
}).mount();
