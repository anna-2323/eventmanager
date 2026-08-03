import { api } from "../core/api.js";
import { $ } from "../core/dom.js";

export async function checkProfile() {
  const user = await api.auth.getUser();
  if (user.logged_in) {
    $("#login-btn").innerHTML = `
                <div class="navbar-item has-dropdown is-hoverable">
                    <a class="navbar-link">
                        <span class="icon"><i class="fas fa-user"></i></span>
                        <span>${user.email}</span>
                    </a>
                    <div class="navbar-dropdown is-right">
                        ${user.role == 0 ? '<a class="navbar-item" href="/admin">Контролен панел</a>' : ""}
                        <a class="navbar-item" href="/profile">Профил</a>
                        <hr class="navbar-divider">
                        <a class="navbar-item" id="logout-btn">Изход</a>
                    </div>
                </div>
            `;
    $("#logout-btn").addEventListener("click", () => {
      fetch("/api/logout", { method: "POST" }).then(() =>
        window.location.reload(),
      );
    });
  }
}
