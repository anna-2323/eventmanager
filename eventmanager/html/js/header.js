import { checkProfile } from "./checkProfile.js";
import { search } from "./search.js";

export function header() {
  document.getElementById("header").innerHTML = `
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
                        <a class="navbar-item" href="#">Спорт</a>
                        <a class="navbar-item" href="#">Театър</a>
                        <a class="navbar-item" href="#">Концерти</a>
                        <a class="navbar-item" href="#">Семинари</a>
                    </div>
                </div>

                <div class="navbar-item has-dropdown navbar-hoverable is-hoverable">
                    <a class="navbar-link navbar-hoverable" href="#">
                        Градове
                    </a>
                    <div class="navbar-dropdown">
                        <a class="navbar-item" href="#">Бургас</a>
                        <a class="navbar-item" href="#">Варна</a>
                        <a class="navbar-item" href="#">Пловдив</a>
                        <a class="navbar-item" href="#">Русе</a>
                        <a class="navbar-item" href="#">София</a>
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
