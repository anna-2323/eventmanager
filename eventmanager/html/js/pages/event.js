import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { header } from "../components/header.js";

header();

const id = window.location.pathname.split("/").pop();
const e = await api.events.get(id);
$("#event").innerHTML = `<div class="box">
        <div class="columns is-vcentered">
            <div class="column is-narrow">
                <figure class="image is-3by4">
                    <img class="event-img" src="${e.img_path}" alt="${e.title}">
                </figure>
            </div>
            <div class="column">
                <h1 class="title mb-2">${e.title}</h1>
                <p class="is-size-6 has-text-grey mb-1">
                    <span class="icon-text">
                        <span class="icon"><i class="fas fa-location-dot"></i></span>
                        <span>${e.venue_name}</span>
                    </span>
                </p>
                <p class="is-size-6 has-text-grey mb-3">
                    <span class="icon-text">
                        <span class="icon"><i class="fas fa-clock"></i></span>
                        <span>${new Date(e.begins_at).toLocaleString("bg-BG")}</span>
                    </span>
                </p>
                <p class="is-size-4 has-text-weight-bold has-text-primary mb-4">
                    ${e.price.toLocaleString("bg-BG", { style: "currency", currency: "BGN" })}
                </p>
                ${
                  e.seats_left > 0
                    ? `<a class="button is-primary" href="/purchase/${e.id}">Купи билет</a>`
                    : `<button class="button is-danger" disabled>Няма свободни места</button>`
                }
            </div>
        </div>
    </div>
    `;
