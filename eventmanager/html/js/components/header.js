import { api } from "../core/api.js";
import { $ } from "../core/dom.js";
import { checkProfile } from "./auth.js";
import { search } from "./search.js";

export async function header() {
    const cities = await api.cities.list();
    const categories = await api.categories.list();
  $("#header").innerHTML = `
    <nav class="navbar" role="navigation" aria-label="main navigation">

        <div class="navbar-brand">
            <a class="navbar-item navbar-hoverable" href="/home">
                <img id="navbar-brand-img" src="/res/event-icon-dark.png" />
                EventMGR
            </a>
            <a role="button" class="navbar-burger" aria-label="menu" aria-expanded="false" data-target="navbarBasicExample">
                <span aria-hidden="true"></span>
                <span aria-hidden="true"></span>
                <span aria-hidden="true"></span>
                <span aria-hidden="true"></span>
            </a>
        </div>

        <div class="navbar-menu">

            <div class="navbar-start">
                <a class="navbar-item navbar-hoverable" href="/events">
                    Всички събития
                </a>

                <div class="navbar-item has-dropdown navbar-hoverable is-hoverable">
                    <a class="navbar-link navbar-hoverable" href="#">
                        Категории
                    </a>
                    <div class="navbar-dropdown">
                        ${categories.map((c) =>
                            `<a class="navbar-item" href="/events?category=${c.id}">${c.title}</a>`
                        ).join("")}
                    </div>
                </div>

                <div class="navbar-item has-dropdown navbar-hoverable is-hoverable">
                    <a class="navbar-link navbar-hoverable" href="#">
                        Градове
                    </a>
                    <div class="navbar-dropdown">
                        ${cities.map((c) => 
                            `<a class="navbar-item" href="/events?city=${c}">${c}</a>`
                        ).join("")}
                    </div>
                </div>
            </div>
        </div>

        <div class="navbar-end">
            <div class="navbar-item">
                <div class="field has-addons">
                    <div class="control has-icons-left">
                        <input class="input" type="search">
                        <span class="icon is-left">
                            <i class="fas fa-search"></i>
                        </span>
                    </div>
                    <div class="control">
                        <button id="search-btn" class="button is-link">Търси</button>
                    </div>
                </div>
            </div>
            <div class="navbar-item" id="login-btn">
                <a class="button is-light" href="/login">
                    <span class="icon"><i class="fas fa-user"></i></span>
                    <span>Вход</span>
                </a>
            </div>
        </div>
    </nav>`;

  search();
  checkProfile();
}
